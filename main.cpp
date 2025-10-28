/*
 * This file is part of Connect4 Game Solver <http://connect4.gamesolver.org>
 * Copyright (C) 2017-2019 Pascal Pons <contact@gamesolver.org>
 *
 * Connect4 Game Solver is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Connect4 Game Solver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with Connect4 Game Solver. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Solver.hpp"
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <ctime>
#include <cstdlib>

using namespace GameSolver::Connect4;
void openingMoves(int fd, std::string& line, std::string& gameHistory, Position& P);
void playMove(int fd, std::string line, Position& P);

int main(int argc, char **argv)
{
  Solver solver;
  std::string line;
  std::string gameHistory;
  std::vector<int> columns;
  Position P;
  std::srand(time(0));

  // open UART
  int fd = open("/dev/serial0", O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) { perror("open /dev/serial0"); return 1; }

  struct termios tty;
  if (tcgetattr(fd, &tty) < 0) return -1;
  cfsetispeed(&tty, B115200);
  cfsetospeed(&tty, B115200);
  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;
  tty.c_cc[VMIN] = 0;
  tty.c_cc[VTIME] = 1;
  if (tcsetattr(fd, TCSANOW, &tty)) return -1;
  tcflush(fd, TCIOFLUSH);
  usleep(100000); // 100ms

  // wait for STM
  char c;
  for (;;) if (read(fd, &c, 1) == 1 && c == 'r') break;
  std::cout << "Ready!\n";
  tcflush(fd, TCIOFLUSH);
  usleep(200000); // 200ms

  // initialize first move to 4 (center column)
  P.play("4"); playMove(fd, "4", P);
  gameHistory = "4";

  // autonomous play
  for (int l = 1;; l++)
  {
    if (read(fd, &c, 1) != 1) continue; // wait until one digit is received
    
    std::string line;
    if (c == '0') line = "reset";
    else if (c >= '1' && c <= '7') line.assign(1, c);
    else continue;

    if (line == "reset")
    {
      std::cout << "Resetting\n";
      P = Position();
      gameHistory.clear();
      for (;;) if (read(fd, &c, 1) == 1 && c == 'r') break;
      std::cout << "Ready!\n";
      usleep(200000); // 200ms
      
      // initialize first move to 4 (center column)
      P.play("4"); playMove(fd, "4", P);
      gameHistory = "4";
      std::srand(time(0));
    }
    else if (P.play(line) != line.size())
    {
      std::cerr << "Line " << l << ": Invalid move " << (P.nbMoves() + 1) << " \"" << line << "\"" << std::endl;
    }
    else
    {
      gameHistory += line;
      std::cout << gameHistory << "\n";
      
      if (gameHistory.size() <= 4) openingMoves(fd, line, gameHistory, P);
      else 
      {
        std::vector<int> scores = solver.analyze(P);
        for (int i = 0; i < Position::WIDTH; i++)
          std::cout << " " << scores[i];

        auto maxScore = std::max_element(scores.begin(), scores.end());
        std::cout << " Max: " << *maxScore;
        std::cout << " Column: ";

        columns.clear(); // reset column vector
        for (size_t i = 0; i < scores.size(); i++)
        {
          if (scores[i] == *maxScore)
          {
            columns.push_back(i + 1);
            std::cout << i + 1 << " ";
          }
        }

        // play best move
        if (!columns.empty())
        {
          if (columns.size() > 1)
          {
            int assignedIndex = std::rand() % columns.size(); // generate random numbers equal to # columns
            P.play(std::to_string(columns[assignedIndex])); playMove(fd, std::to_string(columns[assignedIndex]), P);
            std::cout << "\nPlaying: " << columns[assignedIndex] << "\n";
            gameHistory += std::to_string(columns[assignedIndex]);
          }
          else
          {
            P.play(std::to_string(columns[0])); playMove(fd, std::to_string(columns[0]), P);
            gameHistory += std::to_string(columns[0]);
            std::cout << "\nPlaying: " << columns[0] << "\n";
          }
        }
        std::cout << std::endl;
      }
    }
  }
}

void playMove(int fd, std::string line, Position& P) { 
  char playColumn = '0' + std::stoi(line);
  char gameState = P.canWinNext() ? 'w' : 'm';

  uint8_t out[2] = {static_cast<uint8_t>(playColumn), static_cast<uint8_t>(gameState)};
  std::cout << playColumn << gameState << "\n";

  for (;;)
  {
    ssize_t n = write(fd, out, 2);
    std::cout << "Pinging...\n";

    char c;
    if (read(fd, &c, 1) == 1 && c == 'i') 
    {
      std::cout << "Received!\n";
      tcdrain(fd);
      break;
    }
    usleep(200000);
  }
}

void openingMoves(int fd, std::string& line, std::string& gameHistory, Position& P) {
  if (gameHistory.size() <= 2)
  {
    switch (std::stoi(gameHistory))
    {
    case 41:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 42:
      P.play("2"); playMove(fd, "2", P); 
      gameHistory += "2";
      break;
    case 43:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 44:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 45:
      P.play("2"); playMove(fd, "2", P); 
      gameHistory += "2";
      break;
    case 46:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 47:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    }
    std::cout << gameHistory << "\n";
  }
  else 
  {
    switch (std::stoi(gameHistory))
    {
    case 4141:
      P.play("5"); playMove(fd, "5", P); 
      gameHistory += "5";
      break;
    case 4142:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 4143:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4144:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4145:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4146:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4147:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4221:
      P.play("5"); playMove(fd, "5", P); 
      gameHistory += "5";
      break;
    case 4222:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4223:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 4224:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4225:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4226:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4227:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4361:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4362:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4363:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 4364:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4365:
      P.play("5"); playMove(fd, "5", P); 
      gameHistory += "5";
      break;
    case 4366:
      P.play("7"); playMove(fd, "7", P); 
      gameHistory += "7";
      break;
    case 4367:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 4441:
      P.play("5"); playMove(fd, "5", P); 
      gameHistory += "5";
      break;
    case 4442:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4443:
      P.play("3"); playMove(fd, "3", P); 
      gameHistory += "3";
      break;
    case 4444:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4445:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4446:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4447:
      P.play("3"); playMove(fd, "3", P); 
      gameHistory += "3";
      break;
    case 4521:
      P.play("2"); playMove(fd, "2", P); 
      gameHistory += "2";
      break;
    case 4522:
      P.play("1"); playMove(fd, "1", P); 
      gameHistory += "1";
      break;
    case 4523:
      P.play("3"); playMove(fd, "3", P); 
      gameHistory += "3";
      break;
    case 4524:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4525:
      P.play("2"); playMove(fd, "2", P); 
      gameHistory += "2";
      break;
    case 4526:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4527:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4661:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4662:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4663:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4664:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4665:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 4666:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4667:
      P.play("6"); playMove(fd, "6", P); 
      gameHistory += "6";
      break;
    case 4741:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4742:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4743:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4744:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4745:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4746:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    case 4747:
      P.play("4"); playMove(fd, "4", P); 
      gameHistory += "4";
      break;
    }
    std::cout << gameHistory << "\n\n";
  }
  line.clear();
}
