#include <iostream>
#include <cstring>
#include <cassert>

enum Color {
	EDGE = 0,
	RED,
	BLUE,
	NONE,
	EDGE_RED,
	EDGE_BLUE,
};

class Board {
public:

	// this can be implemented using lambdas but i need stl for <functional> xd
#define FOR_EACH_POS() \
	for (int c = FIRST_COL; c < this->size + FIRST_COL; c++) { \
		for (int r = FIRST_ROW; r < this->size * 2 + FIRST_ROW; r++)

#define END_FOR_EACH_POS }
#define SIDES_NUM 6

	static const size_t FIRST_ROW = 2;
	static const size_t FIRST_COL = 1;

	static const size_t MAX_COL = 11 + FIRST_COL * 2;
	static const size_t MAX_ROW = 11 + 10 + FIRST_ROW * 2;

	static Color otherPlayer(Color cl) {
		switch (cl) {
		case RED:  return BLUE;
		case BLUE: return RED;
		default: assert(0 && "unreachable");
		}
		return NONE;
	}

	static Color winningEdge(Color cl) {
		switch (cl) {
		case RED:  return EDGE_RED;
		case BLUE: return EDGE_BLUE;
		default: assert(0 && "unreachable");
		}
		return NONE;
	}
	size_t getSize() const { return this->size; }
	size_t getPawnsNum() const { return this->redc + this->bluec; }
	size_t getFieldsNum() const { return this->size * this->size; }
	size_t getCount(Color cl) const {
		switch (cl) {
		case RED:  return this->redc;
		case BLUE: return this->bluec;
		default: return -1;
		}
	}
	friend std::istream &operator>>(std::istream &in, Board &board);
	void setField(size_t col, size_t row, Color cl) {
		switch (cl) {
		case RED: this->redc++; break;
		case BLUE: this->bluec++; break;
		case EDGE_RED:
		case EDGE_BLUE:
			break;
		}
		this->fields[col][row] = cl;
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


	bool wins(Color cl) const {
		bool visited[MAX_COL][MAX_ROW] = {false};
		for (int i = 0; i < this->size; i++) {
			size_t c, r;
			switch (cl) { // sides that touch to the first winning edge
			case RED:  c = 1;     r = i + 2; break;
			case BLUE: c = i + 1; r = i + 2; break;
			default: assert(0 && "unreachable"); break;
			}
		        if (visited[c][r] || this->fields[c][r] != cl) continue;
			if (wins_aux(cl, c, r, visited)) return true;
		}
		return false;
	}

	bool wins_aux(Color cl, size_t col, size_t row, bool visited[][MAX_ROW]) const {
		visited[col][row] = true;
		for (int i = 0; i < SIDES_NUM; i++) {
			int r = getKinjoRow(row, i);
			int c = getKinjoCol(col, i);
			// until finds the second winning edge
			if (this->fields[c][r] == winningEdge(cl)) return true;
			if (visited[c][r] || this->fields[c][r] != cl) continue;
			if (wins_aux(cl, c, r, visited)) return true;
		}
		return false;
	}

	Color whoseTurn() {
		return getCount(RED) > getCount(BLUE) ? BLUE : RED;
	}

	bool filled() {
		return getPawnsNum() == size * size;
	}

	bool tryBuildPath(Color cl, int moves) {
		if (wins(cl)) return moves == 0;
		if (moves <= 0) return false;
		if (wins(cl) || filled()) return false;
		FOR_EACH_POS() {
			if (fields[c][r] != NONE) continue;
			setField(c, r, cl);
			bool res = tryBuildPath(cl, moves - 1);
			unsetField(c, r);
			if (res) return true;
		} END_FOR_EACH_POS;
		return false;
	}

	bool canWinNaive(Color cl, int moves) {
		if (wins(cl)) return false; // wins in 0 moves
		if (wins(otherPlayer(cl))) return false;
		if (!tryBuildPath(cl, moves)) return false;
		Color current = whoseTurn();
		int executed_other = 0;
		int needed = moves;
		do {		// simulate game
			if (current == cl) moves--;
			else executed_other++;
			current = otherPlayer(current);
		} while (moves);
		return getFieldsNum() >= getPawnsNum() + needed + executed_other;
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

	Color isGameOver() const {
		if (wins(RED)) return RED;
		if (wins(BLUE)) return BLUE;
		return NONE;
	}

	bool moreThanOnePath(Color cl) {
		FOR_EACH_POS() {
			if (fields[c][r] != cl) continue;
			fields[c][r] = NONE;
			if (!wins(cl)) return false;
			fields[c][r] = cl;
		} END_FOR_EACH_POS;
		return true;
	}

	// check if player can win in EXACTLY n moves, n is 1 or 2
	bool canWinPerfect(Color cl, int moves) {
		if (wins(cl)) return false; // wins in 0 moves
		if (wins(otherPlayer(cl))) return false;
		return miniMax(cl, whoseTurn(), moves);
	}

	// target is player for maximazing, current is current turn player
	// moves is remaining moves
	// false means there is no possibility of target winning
	// true means there is
	bool miniMax(Color target, Color current, int moves) {
		// the target wins in EXACTLY N moves this is the true case
		// if it wins but not in N moves this is dead end
		if (wins(otherPlayer(current))) return target != current && moves == 0;
		if (moves <= 0 || filled()) return false; // the game ends and target doesnt win
		if (target == current) moves--;
		FOR_EACH_POS() {
			if (fields[c][r] != NONE) continue; // only for empty fields
			setField(c, r, current);
			bool res = miniMax(target, otherPlayer(current), moves);
			unsetField(c, r);
			if (target == current) {
				// maximazing players chooses possible winning position
				if (res) return true;
			} else {
				// minimalizing players chooses possible losing position
				if (!res) return false;
			}
		} END_FOR_EACH_POS;
		// target == current -> case where there is no possible winning
		// FALSE - no winning position

		// target != current -> there is no position to make target lose
		// TRUE - target wins
		return target == current ? false : true;
	}

	bool isPossible() {
		if (!isCorrect()) return false;
		bool bwin = wins(BLUE);
		bool rwin = wins(RED);
		if (!(bwin || rwin)) return true;
		if (bwin) {
			if (rwin) return false;
			if (bluec != redc) return false;
			if (moreThanOnePath(BLUE)) return false;
		}
		if (rwin) {
			if (redc <= bluec) return false;
			if (moreThanOnePath(RED)) return false;
		}
		return true;
	}


	void reset() {
		this->redc = 0;
		this->bluec = 0;
		memset(this->fields, 0, MAX_COL * MAX_ROW * sizeof(Color));
	}
private:
	size_t size = 0;
	size_t redc = 0;
	size_t bluec = 0;
	Color fields[MAX_COL][MAX_ROW] = {Color::EDGE};
#undef FOR_EACH_POS
#undef END_FOR_EACH_POS
};

std::istream &operator>>(std::istream &in, Board &board)
{
	size_t first_col = 1;
	size_t row = Board::FIRST_ROW;
	size_t col = Board::FIRST_COL;
	char ch, prev;
	bool inhalf = false;
	Color color = NONE;
	board.reset();
	while (in.get(ch)) {
		if (ch == '<') break;
		prev = ch;
	}
	if (prev == '\n') inhalf = true;
	while (in.get(ch)) {
		switch (ch) {
		case 'r': color = RED; break;
		case 'b': color = BLUE; break;
		case '\n':
			row++;
			if (inhalf) first_col++;
			in.get(ch);
			if (ch == '<') inhalf = true;
			if (inhalf) {
				board.setField(col,           row - 1, EDGE_RED);
				board.setField(first_col - 1, row,     EDGE_BLUE);
			}
			if ((col != 2 || inhalf) && col - first_col == 0) {
				board.size = row / 2;
				while (in.get() != '\n');
				board.setField(col - 1, row, EDGE_BLUE);
				board.setField(col,     row, EDGE_RED);
				return in;
			}
			col = first_col;
			break;
		case '>':
			board.setField(col, row, color);
			col++;
			color = NONE;
			break;
		}
	}
	return in;
}

void handle_cmd(char *input, Board &board) {
	if (!strcmp(input, "BOARD_SIZE")) {
		std::cout << board.getSize() <<  "\n";
		return;
	}
	if (!strcmp(input, "PAWNS_NUMBER")) {
		std::cout << board.getPawnsNum() << "\n";
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
		if (!board.isCorrect()) {
			for (int i = 0; i < 4; i++)
				std::cout << "NO\n";
			return;
		}
		std::cout << (board.canWinNaive(RED, 1)  ? "YES" : "NO") << "\n";
		std::cout << (board.canWinNaive(BLUE, 1) ? "YES" : "NO") << "\n";
		std::cout << (board.canWinNaive(RED, 2)  ? "YES" : "NO") << "\n";
		std::cout << (board.canWinNaive(BLUE, 2) ? "YES" : "NO") << "\n";
		return;
	}
	if (!strcmp(input, "CAN_RED_WIN_IN_1_MOVE_WITH_PERFECT_OPPONENT")) {
		if (!board.isCorrect()) {
			for (int i = 0; i < 4; i++)
				std::cout << "NO\n";
			return;
		}
		std::cout << (board.canWinPerfect(RED, 1)  ? "YES" : "NO") << "\n";
		std::cout << (board.canWinPerfect(BLUE, 1) ? "YES" : "NO") << "\n";
		std::cout << (board.canWinPerfect(RED, 2)  ? "YES" : "NO") << "\n";
		std::cout << (board.canWinPerfect(BLUE, 2) ? "YES" : "NO") << "\n";
		return;
	}
}

int main() {
	Board board;
	char input[124];
	while (std::cin >> board) {
		std::cin >> input;
		handle_cmd(input, board);
		std::cout << "\n";
	}
}
