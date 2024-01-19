#include <curses.h>

#include <iostream>

#include "chess.h"

using namespace std;

void printBoard(const Game& game, bool squareSelected, const Position& selectedSquare);

int main(int argc, char** argv) {
	int ch = '\0';
	MEVENT event;

	initscr();
	if (!has_colors()) {
		endwin();
		cout << "Your terminal does not support colors." << endl;
		return 1;
	}
	start_color();

	noecho();
	cbreak();
	keypad(stdscr, true);
	mousemask(BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED, nullptr);
	init_pair(1, COLOR_WHITE, COLOR_GREEN);
	init_pair(2, COLOR_BLACK, COLOR_RED);

	string fen;
	cout << argc << endl;
	if (argc > 1) {
		fen = argv[1];
		cout << "Loading from fen: " << fen << endl;
	}

	try {
		Game game = fen == "" ? Game() : Game(fen);
		bool squareSelected = false, promoting = false;
		Position selectedSquare, promotionSquare;
		string error;

		do {
			switch (ch) {
				case KEY_MOUSE:
					if (getmouse(&event) == OK) {
						if (event.bstate & BUTTON1_CLICKED) {
							if (event.y / 3 < 8 && event.x / 6 < 8) {
								Position pos = {.file = (Files)(event.x / 6), .rank = (uint)(8 - (event.y / 3))};
								if (squareSelected) {
									if (pos == selectedSquare) {
										squareSelected = false;
									} else {
										try {
											if ((promoting = game.move({.from = selectedSquare, .to = pos}))) {
												promotionSquare = pos;
											}
											squareSelected = false;
											error = "";
										} catch (const runtime_error& e) {
											error = e.what();
										} catch (...) {
											error = "Unknown error";
										}
									}

								} else {
									squareSelected = true;
									selectedSquare = pos;
									error = "";
								}
							} else if (promoting) {
								if (event.y == 1 && event.x >= 32 && event.y < 37) {
									game.promote(promotionSquare, PieceTypes::QUEEN);
									promoting = false;
								} else if (event.y == 2 && event.x >= 32 && event.y < 36) {
									game.promote(promotionSquare, PieceTypes::ROOK);
									promoting = false;
								} else if (event.y == 3 && event.x >= 32 && event.y < 38) {
									game.promote(promotionSquare, PieceTypes::BISHOP);
									promoting = false;
								} else if (event.y == 4 && event.x >= 32 && event.y < 38) {
									game.promote(promotionSquare, PieceTypes::KNIGHT);
									promoting = false;
								}
							}
						} else if (event.bstate & BUTTON2_CLICKED || event.bstate & BUTTON3_CLICKED) {
							squareSelected = false;
							error = "";
						}
					}
					break;
				case '\0':
					break;
				default:
					goto skip;
			}

			clear();
			printBoard(game, squareSelected, selectedSquare);
			if (squareSelected) {
				mvprintw(24, 0, to_string(selectedSquare).c_str());
			}
			mvprintw(25, 0,
					 game.turn() == Players::WHITE ? game.isChecked() ? "White to move (check!)" : "White to move"
												   : game.isChecked() ? "Black to move (check!)" : "Black to move");
			if (error != "") {
				attron(COLOR_PAIR(2));
				mvprintw(26, 0, error.c_str());
				attroff(COLOR_PAIR(2));
			}
			if (promoting) {
				mvprintw(0, 64, "Select promotion piece");
				mvprintw(1, 64, "Queen");
				mvprintw(2, 64, "Rook");
				mvprintw(3, 64, "Bishop");
				mvprintw(4, 64, "Knight");
			}
			mvprintw(30, 0, "White is uppercase, black is lowercase");
			mvprintw(31, 0, "Click to select a square, click again to move");
			mvprintw(32, 0, "Middle/right click to deselect");
			refresh();
		skip:
			void(0);
		} while ((ch = getch()) != 'q');
	} catch (const runtime_error& e) {
		endwin();
		cout << "Error: " << e.what() << endl;
		return 1;

	} catch (...) {
		endwin();
		cout << "Unknown error occured" << endl;
		return 1;
	}

	endwin();

	return 0;
}

void printBoard(const Game& game, bool squareSelected, const Position& selectedSquare) {
	bool black = true;

	vector<Move> availableMoves;

	if (squareSelected) {
		for (const Move& move : game.getAvailableMoves()) {
			if (move.from == selectedSquare) {
				availableMoves.push_back(move);
			}
		}
	}

	for (Files file : FILES) {
		for (uint rank = 1; rank <= 8; rank++) {
			Position square = {.file = file, .rank = rank};

			bool isMoveTarget = false;
			for (const Move& move : availableMoves) {
				if (move.to == square) {
					isMoveTarget = true;
					break;
				}
			}

			if (game.hasPiece(square)) {
				try {
					Piece piece = game.getPiece(square);

					if (squareSelected && (square == selectedSquare || isMoveTarget)) {
						if (square == selectedSquare)
							attron(COLOR_PAIR(1));
						else if (isMoveTarget)
							attron(COLOR_PAIR(2));
					} else if (game.isChecked() && piece.type() == PieceTypes::KING && piece.player() == game.turn())
						attron(COLOR_PAIR(2));
					else if (!black)
						attron(A_REVERSE);

					for (int i = 0; i < 3; i++) {
						if (i == 1) {
							mvprintw((8 - rank) * 3 + 1, file * 6, "  ");
							addch(piece.symbol());
							printw("   ");
						} else {
							mvprintw((8 - rank) * 3 + i, file * 6, "      ");
						}
					}

					attrset(A_NORMAL);
				} catch (const runtime_error& e) {
					throw runtime_error("Rendering error at position " + to_string(square) + ": " + string(e.what()));
				} catch (...) {
					throw runtime_error("Unknown rendering error");
				}
			} else {
				if (squareSelected && (square == selectedSquare || isMoveTarget))
					attron(COLOR_PAIR(1));
				else if (!black)
					attron(A_REVERSE);

				for (int i = 0; i < 3; i++) {
					mvprintw((8 - rank) * 3 + i, file * 6, "      ");
				}

				attrset(A_NORMAL);
			}

			black = !black;
		}

		black = !black;
	}
}