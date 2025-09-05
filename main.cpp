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
#include <iostream>
#include <algorithm>
#include <iterator>
#include <ctime>

using namespace GameSolver::Connect4;

/**
 * Main function.
 * Reads Connect 4 positions, line by line, from standard input
 * and writes one line per position to standard output containing:
 *  - score of the position
 *  - number of nodes explored
 *  - time spent in microsecond to solve the position.
 *
 *  Any invalid position (invalid sequence of move, or already won game)
 *  will generate an error message to standard error and an empty line to standard output.
 */
int main(int argc, char **argv)
{
  Solver solver;
  bool weak = false;
  bool analyze = false;

  std::string opening_book = "7x6.book";
  for (int i = 1; i < argc; i++)
  {
    if (argv[i][0] == '-')
    {
      if (argv[i][1] == 'w')
        weak = true; // parameter -w: use weak solver
      else if (argv[i][1] == 'b')
      { // paramater -b: define an alternative opening book
        if (++i < argc)
          opening_book = std::string(argv[i]);
      }
      else if (argv[i][1] == 'a')
      { // paramater -a: make an analysis of all possible moves
        analyze = true;
      }
    }
  }
  solver.loadBook(opening_book);

  std::string line;
  std::string gameHistory;
  std::vector<int> columns;
  Position P;
  std::srand(time(0));

  // initialize first move to 4 (center column)
  P.play("4");
  gameHistory = "4";
  // test input
  P.play("3");
  gameHistory += "3";
  std::cout << gameHistory << "\n";

  // opening
  switch (std::stoi(gameHistory))
  {
  case 41:
    P.play("4"); gameHistory += "4";
    break;
  case 42:
    P.play("2"); gameHistory += "2";
    break;
  case 43:
    P.play("6"); gameHistory += "6";
    break;
  case 44:
    P.play("4"); gameHistory += "4";
    break;
  case 45:
    P.play("2"); gameHistory += "2";
    break;
  case 46:
    P.play("6"); gameHistory += "6";
    break;
  case 47:
    P.play("7"); gameHistory += "7";
    break;
  }
  std::cout << gameHistory << "\n";

  // test input
  P.play("4");
  gameHistory += "4";
  std::cout << gameHistory << "\n";

  switch (std::stoi(gameHistory))
  {
  case 4141:
    P.play("5"); gameHistory += "5";
    break;
  case 4142:
    P.play("6"); gameHistory += "6";
    break;
  case 4143:
    P.play("4"); gameHistory += "4";
    break;
  case 4144:
    P.play("4"); gameHistory += "4";
    break;
  case 4145:
    P.play("4"); gameHistory += "4";
    break;
  case 4146:
    P.play("4"); gameHistory += "4";
    break;
  case 4147:
    P.play("4"); gameHistory += "4";
    break;
  case 4221:
    P.play("5"); gameHistory += "5";
    break;
  case 4222:
    P.play("4"); gameHistory += "4";
    break;
  case 4223:
    P.play("6"); gameHistory += "6";
    break;
  case 4224:
    P.play("4"); gameHistory += "4";
    break;
  case 4225:
    P.play("4"); gameHistory += "4";
    break;
  case 4226:
    P.play("4"); gameHistory += "4";
    break;
  case 4227:
    P.play("4"); gameHistory += "4";
    break;
  case 4361:
    P.play("4"); gameHistory += "4";
    break;
  case 4362:
    P.play("4"); gameHistory += "4";
    break;
  case 4363:
    P.play("6"); gameHistory += "6";
    break;
  case 4364:
    P.play("4"); gameHistory += "4";
    break;
  case 4365:
    P.play("5"); gameHistory += "5";
    break;
  case 4366:
    P.play("7"); gameHistory += "7";
    break;
  case 4367:
    P.play("6"); gameHistory += "6";
    break;
  case 4441:
    P.play("5"); gameHistory += "5";
    break;
  case 4442:
    P.play("4"); gameHistory += "4";
    break;
  case 4443:
    P.play("3"); gameHistory += "3";
    break;
  case 4444:
    P.play("4"); gameHistory += "4";
    break;
  case 4445:
    P.play("4"); gameHistory += "4";
    break;
  case 4446:
    P.play("4"); gameHistory += "4";
    break;
  case 4447:
    P.play("3"); gameHistory += "3";
    break;
  case 4521:
    P.play("2"); gameHistory += "2";
    break;
  case 4522:
    P.play("1"); gameHistory += "1";
    break;
  case 4523:
    P.play("3"); gameHistory += "3";
    break;
  case 4524:
    P.play("4"); gameHistory += "4";
    break;
  case 4525:
    P.play("2"); gameHistory += "2";
    break;
  case 4526:
    P.play("4"); gameHistory += "4";
    break;
  case 4527:
    P.play("4"); gameHistory += "4";
    break;
  case 4661:
    P.play("4"); gameHistory += "4";
    break;
  case 4662:
    P.play("4"); gameHistory += "4";
    break;
  case 4663:
    P.play("4"); gameHistory += "4";
    break;
  case 4664:
    P.play("4"); gameHistory += "4";
    break;
  case 4665:
    P.play("6"); gameHistory += "6";
    break;
  case 4666:
    P.play("4"); gameHistory += "4";
    break;
  case 4667:
    P.play("6"); gameHistory += "6";
    break;
  case 4741:
    P.play("4"); gameHistory += "4";
    break;
  case 4742:
    P.play("4"); gameHistory += "4";
    break;
  case 4743:
    P.play("4"); gameHistory += "4";
    break;
  case 4744:
    P.play("4"); gameHistory += "4";
    break;
  case 4745:
    P.play("4"); gameHistory += "4";
    break;
  case 4746:
    P.play("4"); gameHistory += "4";
    break;
  case 4747:
    P.play("4"); gameHistory += "4";
    break;
  }
  std::cout << gameHistory << "\n";
  line.clear();

  // autonomous play
  for (int l = 1; std::getline(std::cin, line); l++)
  {
    if (line == "reset")
    {
      std::cout << "Resetting\n";
      P = Position();
      gameHistory.clear();

      P.play("4");
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
      std::cout << gameHistory;

      if (analyze)
      {
        std::vector<int> scores = solver.analyze(P, weak);
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
            P.play(std::to_string(columns[assignedIndex]));
            gameHistory += std::to_string(columns[assignedIndex]);
          }
          else
          {
            P.play(std::to_string(columns[0]));
            gameHistory += std::to_string(columns[0]);
          }
        }
      }
      else
      {
        int score = solver.solve(P, weak);
        std::cout << " " << score;
      }
      std::cout << std::endl;
    }
  }
}
