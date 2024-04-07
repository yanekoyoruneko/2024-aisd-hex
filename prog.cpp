#include <iostream>
#include <cstring>
#include <cassert>


enum Player {
	EDGE = 0,
	RED,
	BLUE,
	NONE,
	EDGE_R1,
	EDGE_R2,
	EDGE_B1,
	EDGE_B2,
};

class Board {
public:
	static const size_t FIRST_ROW = 2;
	static const size_t FIRST_COL = 1;

	static const size_t MAX_COL = 11 + FIRST_COL * 2;
	static const size_t MAX_ROW = 11 + 10 + FIRST_ROW * 2;

	static Player otherPlayer(Player pl) {
		switch (pl) {
		case RED:  return BLUE;
		case BLUE: return RED;
		default: assert(0 && "unreachable");
		}
	}

	static Player winningEdge(Player pl) {
		switch (pl) {
		case RED:  return EDGE_R2;
		case BLUE: return EDGE_B2;
		default: assert(0 && "unreachable");
		}
		return NONE;
	}
	size_t getSize() const { return this->size; }
	size_t getPawnsCount() const { return this->redc + this->bluec; }
	size_t getCount(Player pl) const {
		switch (pl) {
		case RED:  return this->redc;
		case BLUE: return this->bluec;
		default: return -1;
		}
	}
	friend std::istream &operator>>(std::istream &in, Board &board);
	void setField(size_t col, size_t row, Player pl) {
		switch (pl) {
		case RED: this->redc++; break;
		case BLUE: this->bluec++; break;
		case EDGE_R1:
		case EDGE_R2:
		case EDGE_B1:
		case EDGE_B2:
			break;
		}
		this->fields[col][row] = pl;
	}
	void unsetField(size_t col, size_t row) {
		switch (this->fields[col][row]) {
		case RED: this->redc--; break;
		case BLUE: this->bluec--; break;
		}
		this->fields[col][row] = NONE;
	}


	bool isCorrect() const {
		int tmp = (int)this->redc - this->bluec;
		return 0 <= tmp && tmp <= 1;
	}


	bool wins(Player pl) const {
		bool visited[MAX_COL][MAX_ROW] = {false};
		for (int i = 0; i < this->size; i++) {
			size_t c, r;
			switch (pl) { // sides that touch to the first winning edge
			case RED:  c = 1;     r = i + 2; break;
			case BLUE: c = i + 1; r = i + 2; break;
			default: assert(0 && "unreachable"); break;
			}
		        if (visited[c][r] || this->fields[c][r] != pl) continue;
			if (wins_aux(pl, c, r, visited)) return true;
		}
		return false;
	}

	bool wins_aux(Player pl, size_t col, size_t row, bool visited[][MAX_ROW]) const {
		visited[col][row] = true;
		for (int i = 0; i < 6; i++) {
			int r = getKinjoRow(row, i);
			int c = getKinjoCol(col, i);
			// until finds the second winning edge
			if (this->fields[c][r] == winningEdge(pl)) return true;
			if (visited[c][r] || this->fields[c][r] != pl) continue;
			if (wins_aux(pl, c, r, visited)) return true;
		}
		return false;
	}

	// cant type neighboordasdfklj
	static size_t getKinjoRow(size_t row, int side) {
		switch (side) {
		case 0: return row - 2; break;
		case 1: return row - 1; break;
		case 2: return row - 1; break;
		case 3: return row + 1; break;
		case 4: return row + 1; break;
		case 5: return row + 2; break;
		default: assert(0 && "unreachable");
		}
		return 0;
	}

	static size_t getKinjoCol(size_t col, int side) {
		switch (side) {
		case 0: return col - 1; break;
		case 1: return col - 1; break;
		case 2: return col;     break;
		case 3: return col;     break;
		case 4: return col + 1; break;
		case 5: return col + 1; break;
		default: assert(0 && "unreachable");
		}
		return 0;
	}

	Player isGameOver() const {
		if (wins(RED)) return RED;
		if (wins(BLUE)) return BLUE;
		return NONE;
	}

	bool more_than_one_path(Player pl) {
		int found = 0;
		for (int c = 0; c < MAX_COL && !found; c++) {
			for (int r = 0; r < MAX_ROW && !found; r++) {
				if (fields[c][r] == pl) {
					fields[c][r] = NONE;
					if (!wins(pl)) {
						found = 1;
					}
					fields[c][r] = pl;
				}
			}
		}
		return !found;
	}

	bool isPossible() {
		if (!isCorrect()) return false;
		bool bwin = wins(BLUE);
		bool rwin = wins(RED);
		if (!(bwin || rwin)) {
			return true;
		}
		if (bwin && rwin) {
			return false;
		}
		if (bwin && bluec != redc) {
			return false;
		}
		if (rwin && redc <= bluec) {
			return false;
		}
		if (rwin && more_than_one_path(RED)) {
			return false;
		}
		if (bwin && more_than_one_path(BLUE)) {
			return false;
		}
		return true;
	}
	void reset() {
		this->redc = 0;
		this->bluec = 0;
		memset(this->fields, MAX_COL * MAX_ROW * sizeof(Player), 0);
	}
private:
	size_t size = 0;
	size_t redc = 0;
	size_t bluec = 0;
	Player fields[MAX_COL][MAX_ROW] = {Player::EDGE};
};

std::istream &operator>>(std::istream &in, Board &board)
{
	size_t first_col = 1;
	size_t row = Board::FIRST_ROW;
	size_t col = Board::FIRST_COL;
	char ch, prev;
	bool inhalf = false;
	Player last_pl = NONE;
	board.reset();
	board.setField(Board::FIRST_COL - 1, Board::FIRST_ROW - 1, EDGE_R1);
	board.setField(Board::FIRST_COL    , Board::FIRST_ROW - 1, EDGE_B1);
	board.setField(Board::FIRST_COL - 1, Board::FIRST_ROW, EDGE_R1);
	while (std::cin && std::cin.get(ch)) {
		if (ch == '<') break;
		prev = ch;
	}
	if (prev == '\n') inhalf = true;
	while (std::cin.get(ch)) {
		switch (ch) {
		case 'r': last_pl = RED; break;
		case 'b': last_pl = BLUE; break;
		case '\n':
			board.setField(col, row, inhalf ? EDGE_R2 : EDGE_B1);
			row++;
			if ((col != 2 || inhalf) && col - first_col == 1) {
				board.size = row / 2;
				while (std::cin.get() != '\n');
				board.setField(col - 1, row, EDGE_B2);
				board.setField(col,     row, EDGE_R2);
				return std::cin;
			}
			if (inhalf) first_col++;
			std::cin.get(ch);
			if (ch == '<') inhalf = true;
			col = first_col;
			board.setField(col - 1, row, inhalf ? EDGE_B2 : EDGE_R1);
			break;
		case '>':
			board.setField(col, row, last_pl);
			col++;
			last_pl = NONE;
			break;
		}
	}
	return std::cin;
}

void handle_cmd(char *input, Board &board) {
	if (!strcmp(input, "BOARD_SIZE")) {
		std::cout << board.getSize() <<  "\n";
		return;
	}
	if (!strcmp(input, "PAWNS_NUMBER")) {
		std::cout << board.getPawnsCount() << "\n";
		return;
	}
	if (!strcmp(input, "IS_BOARD_CORRECT")) {
		std::cout << (board.isCorrect() ? "YES" : "NO") << "\n";
		return;
	}
	if (!strcmp(input, "IS_GAME_OVER")) {
		if (!board.isCorrect()) {
			printf("NO\n");
			return;
		}
		switch (board.isGameOver()) {
		case RED:  std::cout << "YES RED\n"; break;
		case BLUE: std::cout << "YES BLUE\n"; break;
		case NONE: std::cout << "NO\n"; break;
		default: assert(0 && "unreachable");
		}
		return;
	}
	if (!strcmp(input, "IS_BOARD_POSSIBLE")) {
		std::cout << (board.isPossible() ? "YES" : "NO") << "\n";
		return;
	}
	if (!strcmp(input, "CAN_RED_WIN_IN_1_MOVE_WITH_NAIVE_OPPONENT")) {
		return;
	}
	if (!strcmp(input, "CAN_RED_WIN_IN_1_MOVE_WITH_PERFECT_OPPONENT")) {
		return;
	}
}

int main() {
	Board board;
	char input[124];
	std::cout.sync_with_stdio(false);
	while (std::cin >> board) {
		std::cin >> input;
		handle_cmd(input, board);
		std::cout << "\n";
	}
}
