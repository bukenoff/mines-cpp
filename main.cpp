#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono;      // nanoseconds, system_clock, seconds

// Colors for later usage
// ANSI escape codes for colors
/*const std::string red = "\033[31m";*/
/*const std::string green = "\033[32m";*/
/*const std::string reset = "\033[0m";*/
/**/
/*std::cout << red << "This text is red." << reset << std::endl;*/
/*std::cout << green << "This text is green." << reset << std::endl;*/
/**/

const std::string red = "\033[31m";
const std::string reset = "\033[0m";

using namespace std;

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

  Cell(int r, int c) {
    row = r;
    col = c;
    bombs_around = 0;
    has_bomb = false;
    is_open = false;
    is_flagged = false;
  }
};

class Minesweeper {
  vector<vector<Cell>> board;

public:
  int rows;
  int cols;
  bool game_over;
  int bombs_count;
  Coordinates cursor;

  Minesweeper(int r, int c)
      : rows(r), cols(c), game_over(false), bombs_count(9), cursor(0, 0) {}

  void init() {
    game_over = true;
    makeEmptyBoard();
    renderBoard();
  }

  void start(int start_coordinates) {
    game_over = false;
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

    for (int num : coordinates) {
      cout << "number is: " << num << "\n";
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
          cout << red;
        }

        if (cell.is_open) {
          if (cell.has_bomb) {
            cout << "b";
          } else if (cell.bombs_around) {
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
         << "\n";
  };

  void renderFactualBoard() {
    cout << "+-------------------+"
         << "\n";

    for (const auto &row : board) {
      for (const auto &cell : row) {
        if (cell.col == 0) {
          cout << "| ";
        }

        if (cell.has_bomb) {
          cout << "×";
        } else if (cell.bombs_around) {
          cout << cell.bombs_around;
        } else {
          cout << " ";
        }
        cout << " ";

        if (cell.col == cols - 1) {
          cout << "|";
        }
      }
      cout << "\n";
    }

    cout << "+-------------------+"
         << "\n";
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
    auto &cell = board[row][col];

    if (cell.is_open) {
      return;
    }

    cell.is_open = true;

    if (cell.has_bomb) {
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
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
  mnswpr.start(0);

  while (true) {
    cin >> move;
    cout << "move: " << move << "\n";
    if (move == 'h') {
      mnswpr.cursor.change(mnswpr.cursor.row, mnswpr.cursor.col - 1);
    } else if (move == 'l') {
      mnswpr.cursor.change(mnswpr.cursor.row, mnswpr.cursor.col + 1);
    } else if (move == 'j') {
      mnswpr.cursor.change(mnswpr.cursor.row + 1, mnswpr.cursor.col);
    } else if (move == 'k') {
      mnswpr.cursor.change(mnswpr.cursor.row - 1, mnswpr.cursor.col);
    } else if (move == 'o') {
      mnswpr.open(mnswpr.cursor.row, mnswpr.cursor.col);
    } else if (move == 'f') {
      mnswpr.flag(mnswpr.cursor.row, mnswpr.cursor.col);
    } else if (move == '0') {
      mnswpr.cursor.change(mnswpr.cursor.row, 0);
    } else if (move == '$') {
      mnswpr.cursor.change(mnswpr.cursor.row, mnswpr.cols - 1);
    } else {
      cout << "no such button supported"
           << "\n";
    }
    mnswpr.renderBoard();
  }

  return 0;
}
