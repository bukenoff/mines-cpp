#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <stdlib.h>
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

void clearScreen() { std::cout << "\033[2J\033[H"; }

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
    cout << "k - move up"
         << "\n";
    cout << "j - move down"
         << "\n";
    cout << "h - move left"
         << "\n";
    cout << "l - move right"
         << "\n";
    cout << "o - open cell"
         << "\n";
    cout << "f - put flag"
         << "\n";
  }

  void renderBoard() {
    clearScreen();
    cout << "+-------------------+"
         << "\n";

    for (const auto &row : board) {
      for (const auto &cell : row) {

        if (cell.col == 0) {
          cout << "| ";
        }

        if (cell.row == cursor.row && cell.col == cursor.col) {
          cout << bold;
        }

        if (cell.is_open) {
          if (cell.has_bomb) {
            cout << "b";
          } else if (cell.bombs_around) {
            cout << value_colors[cell.bombs_around];
            cout << cell.bombs_around;
          } else {
            cout << " ";
          }
        } else if (cell.is_flagged) {
          cout << "f";
        } else {
          cout << "~";
        }

        cout << " ";
        cout << reset;
        if (cell.col == cols - 1) {
          cout << "|";
        }
      }
      cout << "\n";
    }

    cout << "+-------------------+"
         << "\n\n";

    if (status == Victory) {
      cout << "You won, congrats"
           << "\n";
      cout << "(˶ᵔ ᵕ ᵔ˶)"
           << "\n\n";
    } else if (status == Loss) {
      cout << "You lost"
           << "\n";
      cout << "(´•︵•`)"
           << "\n\n";
    } else {
      cout << "Keep playing"
           << "\n";
      cout << "( • _ • )"
           << "\n\n";
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
    closed_cells_count -= 1;

    if (closed_cells_count == bombs_count) {
      status = Victory;
      return;
    }

    if (cell.has_bomb) {
      status = Loss;
      cout << "You lost"
           << "\n";
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
  Minesweeper mnswpr(9, 9);
  mnswpr.init();
  char move;

  while (true) {
    cin >> move;
    switch (move) {
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
      cout << "no such button supported"
           << "\n";
    }
    mnswpr.renderBoard();
  }

  return 0;
}
