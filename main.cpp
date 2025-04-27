#include <algorithm>
#include <charconv>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <ncurses.h>
#include <set>
#include <stdlib.h>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

using namespace std::this_thread;
using namespace std::chrono;

const std::string black = "\033[30m";
const std::string white = "\033[37m";
const std::string green = "\033[32m";
const std::string blue = "\033[34m";
const std::string red = "\033[31m";
const std::string yellow = "\033[33m";
const std::string magenta = "\033[35m";
const std::string cyan = "\033[36m";
const std::string reset = "\033[0m";
const std::string bold = "\033[1m";

using namespace std;

map<int, string> value_colors = {
    {1, blue},    {2, green}, {3, red},   {4, yellow},
    {5, magenta}, {6, cyan},  {7, black}, {8, white},
};

class Coordinates {
public:
  int row;
  int col;

  Coordinates(int r, int c) : row(r), col(c) {}

  void change(int r, int c) {
    row = (r < 0 ? 0 : r % 9);
    col = (c < 0 ? 0 : c % 9);
  }
};

class Cell {
public:
  int row;
  int col;
  int bombs_around;
  bool has_bomb;
  bool is_open;
  bool is_flagged;

  Cell(int r, int c)
      : row(r), col(c), bombs_around(0), has_bomb(false), is_open(false),
        is_flagged(false) {}
};

enum Status {
  Pending,
  Active,
  Loss,
  Victory,
};

class Minesweeper {
  vector<vector<Cell>> board;
  int bombs_count;
  int closed_cells_count;

public:
  int rows;
  int cols;
  bool game_over;
  Coordinates cursor;
  Status status;

  Minesweeper(int r, int c)
      : rows(r), cols(c), game_over(false), bombs_count(9),
        closed_cells_count(c * r), cursor(0, 0), status(Pending) {}

  void init() {
    game_over = true;
    makeEmptyBoard();
    renderBoard();
  }

  void start(int start_coordinates) {
    game_over = false;
    status = Active;
    plantBombs(start_coordinates);
    renderBoard();
  }

  void makeEmptyBoard() {
    vector<vector<Cell>> new_board;
    new_board.reserve(rows);

    for (int i = 0; i < rows; i++) {
      vector<Cell> row;
      row.reserve(cols);

      for (int j = 0; j < cols; j++) {
        Cell new_cell(i, j);
        row.push_back(new_cell);
      }

      new_board.push_back(row);
    }

    board = new_board;
  }

  vector<int> getRandomCoordinates(int start_coordinates) {
    vector<int> coordinates;
    int count = 0;
    set<int> generated_numbers;

    srand(time(0));

    while (count < bombs_count) {
      int random_row = rand() % (rows - 1);
      int random_col = rand() % (cols - 1);
      int num = random_row * rows + random_col;

      if (num == start_coordinates) {
        continue;
      }

      if (find(coordinates.begin(), coordinates.end(), num) !=
          coordinates.end()) {
        continue;
      } else {
        coordinates.push_back(num);
        count += 1;
      }
    }

    return coordinates;
  };

  void plantHints(int row, int col) {
    vector<tuple<int, int>> neighbors = getNeighbors(row, col);

    for (const auto neighbor : neighbors) {
      int neighbor_row = get<0>(neighbor);
      int neighbor_col = get<1>(neighbor);

      board[neighbor_row][neighbor_col].bombs_around += 1;
    }
  }

  void plantBombs(int start_coordinates) {
    vector<int> random_coordinates = getRandomCoordinates(start_coordinates);

    while (!random_coordinates.empty()) {
      int coordinates = random_coordinates.back();
      int row = coordinates / rows;
      int col = coordinates % cols;
      random_coordinates.pop_back();

      board[row][col].has_bomb = true;
      plantHints(row, col);
    }
  }

  void printInstructions() {
    mvprintw(14, 0, "k - move up\n");
    mvprintw(15, 0, "j - move down\n");
    mvprintw(16, 0, "h - move left\n");
    mvprintw(17, 0, "l - move right\n");
    mvprintw(18, 0, "o - open cell\n");
    mvprintw(19, 0, "f - put flag\n");
  }

  void renderBoard() {
    mvprintw(0, 0, "+-------------------+\n");

    for (const auto &row : board) {
      for (const auto &cell : row) {
        int curr_row = cell.row + 1;
        int curr_col = cell.col * 2 + 2;

        if (cell.col == 0) {
          mvprintw(cell.row + 1, cell.col, "| ");
        }

        if (cell.row == cursor.row && cell.col == cursor.col) {
          attron(A_UNDERLINE);
        }

        if (cell.is_open) {
          if (cell.has_bomb) {
            mvprintw(curr_row, curr_col, "b");
          } else if (cell.bombs_around) {
            mvprintw(curr_row, curr_col, "%d", cell.bombs_around);
          } else {
            mvprintw(curr_row, curr_col, " ");
          }
        } else if (cell.is_flagged) {
          mvprintw(curr_row, curr_col, "f");
        } else {
          mvprintw(curr_row, curr_col, "~");
        }

        attroff(A_UNDERLINE);

        if (cell.col == cols - 1) {
          mvprintw(curr_row, curr_col + 2, "|\n");
        }
      }
    }

    mvprintw(10, 0, "+-------------------+\n\n");

    if (status == Victory) {
      mvprintw(11, 0, "You won, congrats\n");
      mvprintw(12, 0, "(˶ᵔ ᵕ ᵔ˶)\n");
    } else if (status == Loss) {
      mvprintw(11, 0, "You lost\n");
      mvprintw(12, 0, "(´•︵•`)\n");
    } else {
      mvprintw(11, 0, "Keep playing\n");
      mvprintw(12, 0, "( • _ • )\n\n");
    }
    printInstructions();
  };

  vector<tuple<int, int>> getNeighbors(int row, int col) {
    vector<tuple<int, int>> neighbors;
    neighbors.reserve(8);

    if (row == 0) { // if first row
      if (col == 0) {
        neighbors.push_back(make_tuple(row, col + 1));
        neighbors.push_back(make_tuple(row + 1, col + 1));
        neighbors.push_back(make_tuple(row + 1, col + 1));
      } else if (col == cols - 1) {
        neighbors.push_back(make_tuple(row, col - 1));
        neighbors.push_back(make_tuple(row + 1, col));
        neighbors.push_back(make_tuple(row + 1, col - 1));
      } else {
        neighbors.push_back(make_tuple(row, col + 1));
        neighbors.push_back(make_tuple(row, col - 1));
        neighbors.push_back(make_tuple(row + 1, col + 1));
        neighbors.push_back(make_tuple(row + 1, col));
        neighbors.push_back(make_tuple(row + 1, col - 1));
      }
    } else if (row == rows - 1) { // if last row
      if (col == 0) {
        neighbors.push_back(make_tuple(row, col + 1));
        neighbors.push_back(make_tuple(row - 1, col));
        neighbors.push_back(make_tuple(row - 1, col + 1));
      } else if (col == cols - 1) {
        neighbors.push_back(make_tuple(row, col - 1));
        neighbors.push_back(make_tuple(row - 1, col));
        neighbors.push_back(make_tuple(row - 1, col - 1));
      } else {
        neighbors.push_back(make_tuple(row, col + 1));
        neighbors.push_back(make_tuple(row, col - 1));
        neighbors.push_back(make_tuple(row - 1, col + 1));
        neighbors.push_back(make_tuple(row - 1, col));
        neighbors.push_back(make_tuple(row - 1, col - 1));
      }
    } else { // any other row
      if (col == 0) {
        neighbors.push_back(make_tuple(row, col + 1));
        neighbors.push_back(make_tuple(row - 1, col));
        neighbors.push_back(make_tuple(row - 1, col + 1));
        neighbors.push_back(make_tuple(row + 1, col));
        neighbors.push_back(make_tuple(row + 1, col + 1));
      } else if (col == cols - 1) {
        neighbors.push_back(make_tuple(row, col - 1));
        neighbors.push_back(make_tuple(row - 1, col));
        neighbors.push_back(make_tuple(row - 1, col - 1));
        neighbors.push_back(make_tuple(row + 1, col));
        neighbors.push_back(make_tuple(row + 1, col - 1));
      } else {
        neighbors.push_back(make_tuple(row, col + 1));
        neighbors.push_back(make_tuple(row, col - 1));
        neighbors.push_back(make_tuple(row - 1, col + 1));
        neighbors.push_back(make_tuple(row - 1, col));
        neighbors.push_back(make_tuple(row - 1, col - 1));
        neighbors.push_back(make_tuple(row + 1, col + 1));
        neighbors.push_back(make_tuple(row + 1, col));
        neighbors.push_back(make_tuple(row + 1, col - 1));
      }
    }

    return neighbors;
  };

  void open(int row, int col) {
    // You won already
    if (closed_cells_count == bombs_count) {
      return;
    }
    auto &cell = board[row][col];

    if (cell.is_open || cell.is_flagged) {
      return;
    }

    cell.is_open = true;

    if (cell.has_bomb) {
      status = Loss;
      mvprintw(20, 0, "You lost\n");
      return;
    }

    closed_cells_count -= 1;

    if (closed_cells_count == bombs_count) {
      status = Victory;
      return;
    }

    if (!cell.bombs_around) {
      vector<tuple<int, int>> neighbors = getNeighbors(row, col);

      for (const auto neighbor : neighbors) {
        int neighbor_row = get<0>(neighbor);
        int neighbor_col = get<1>(neighbor);

        open(neighbor_row, neighbor_col);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        renderBoard();
      }
    }
  }

  void flag(int row, int col) {
    auto &cell = board[row][col];
    cell.is_flagged = !cell.is_flagged;
  }
};

int main() {
  initscr();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  curs_set(0);

  Minesweeper mnswpr(9, 9);
  mnswpr.init();

  const std::chrono::milliseconds target_frame_duration(1000 / 60);

  while (true) {
    int key = getch();
    auto frame_start_time = std::chrono::steady_clock::now();

    switch (key) {
    case 'h':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      mnswpr.cursor.change(mnswpr.cursor.row, mnswpr.cursor.col - 1);
      break;
    case 'l':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      mnswpr.cursor.change(mnswpr.cursor.row, mnswpr.cursor.col + 1);
      break;
    case 'j':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      mnswpr.cursor.change(mnswpr.cursor.row + 1, mnswpr.cursor.col);
      break;
    case 'k':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      mnswpr.cursor.change(mnswpr.cursor.row - 1, mnswpr.cursor.col);
      break;
    case 'o':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      if (mnswpr.status == Pending) {
        int start_point = mnswpr.cursor.row * mnswpr.rows + mnswpr.cursor.col;
        mnswpr.start(start_point);
      }
      mnswpr.open(mnswpr.cursor.row, mnswpr.cursor.col);
      break;
    case 'f':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      mnswpr.flag(mnswpr.cursor.row, mnswpr.cursor.col);
      break;
    case '0':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      mnswpr.cursor.change(mnswpr.cursor.row, 0);
      break;
    case '$':
      if (mnswpr.status == Loss || mnswpr.status == Victory) {
        break;
      }
      mnswpr.cursor.change(mnswpr.cursor.row, mnswpr.cols - 1);
      break;
    default:
      mvprintw(22, 0, "no such button supported\n");
    }

    refresh();
    mnswpr.renderBoard();
    // --- Timing (continued) ---
    auto frame_end_time = std::chrono::steady_clock::now();
    auto frame_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        frame_end_time - frame_start_time);

    if (frame_duration < target_frame_duration) {
      std::this_thread::sleep_for(target_frame_duration - frame_duration);
    }
  }

  endwin();

  return 0;
}
