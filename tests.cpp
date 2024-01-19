#define CATCH_CONFIG_MAIN

#include <lib/catch.hpp>

#include "chess.h"

using namespace std;

TEST_CASE("Board basics") {
	Board board;

	SECTION("Basic construction") {
		for (int i = 0; i < 8; i++) {
			board[A][i + 1] = i * 8;
			board[B][i + 1] = i * 8 + 1;
			board[C][i + 1] = i * 8 + 2;
			board[D][i + 1] = i * 8 + 3;
			board[E][i + 1] = i * 8 + 4;
			board[F][i + 1] = i * 8 + 5;
			board[G][i + 1] = i * 8 + 6;
			board[H][i + 1] = i * 8 + 7;
		}

		for (int i = 0; i < 64; i++) {
			REQUIRE(board.get(i / 8, i % 8) == i);
		}
	}
}

TEST_CASE("Game basics") {
	Game game;

	SECTION("Basic setup") {
		for (const Files file : FILES) {
			Position wPos = {.file = file, .rank = 2};
			Position bPos = {.file = file, .rank = 7};

			REQUIRE(game.hasPiece(wPos));
			REQUIRE(game.hasPiece(bPos));

			Piece wPawn = game.getPiece(wPos);
			Piece bPawn = game.getPiece(bPos);

			REQUIRE(wPawn.type() == PieceTypes::PAWN);
			REQUIRE(wPawn.player() == Players::WHITE);
			REQUIRE(bPawn.type() == PieceTypes::PAWN);
			REQUIRE(bPawn.player() == Players::BLACK);

			if (file == Files::A || file == Files::H) {
				Position wPos = {.file = file, .rank = 1};
				Position bPos = {.file = file, .rank = 8};

				REQUIRE(game.hasPiece(wPos));
				REQUIRE(game.hasPiece(bPos));

				Piece wRook = game.getPiece(wPos);
				Piece bRook = game.getPiece(bPos);

				REQUIRE(wRook.type() == PieceTypes::ROOK);
				REQUIRE(wRook.player() == Players::WHITE);
				REQUIRE(bRook.type() == PieceTypes::ROOK);
				REQUIRE(bRook.player() == Players::BLACK);
			} else if (file == Files::B || file == Files::G) {
				Position wPos = {.file = file, .rank = 1};
				Position bPos = {.file = file, .rank = 8};

				REQUIRE(game.hasPiece(wPos));
				REQUIRE(game.hasPiece(bPos));

				Piece wKnight = game.getPiece(wPos);
				Piece bKnight = game.getPiece(bPos);

				REQUIRE(wKnight.type() == PieceTypes::KNIGHT);
				REQUIRE(wKnight.player() == Players::WHITE);
				REQUIRE(bKnight.type() == PieceTypes::KNIGHT);
				REQUIRE(bKnight.player() == Players::BLACK);
			} else if (file == Files::C || file == Files::F) {
				Position wPos = {.file = file, .rank = 1};
				Position bPos = {.file = file, .rank = 8};

				REQUIRE(game.hasPiece(wPos));
				REQUIRE(game.hasPiece(bPos));

				Piece wBishop = game.getPiece(wPos);
				Piece bBishop = game.getPiece(bPos);

				REQUIRE(wBishop.type() == PieceTypes::BISHOP);
				REQUIRE(wBishop.player() == Players::WHITE);
				REQUIRE(bBishop.type() == PieceTypes::BISHOP);
				REQUIRE(bBishop.player() == Players::BLACK);
			} else if (file == Files::D) {
				Position wPos = {.file = file, .rank = 1};
				Position bPos = {.file = file, .rank = 8};

				REQUIRE(game.hasPiece(wPos));
				REQUIRE(game.hasPiece(bPos));

				Piece wQueen = game.getPiece(wPos);
				Piece bQueen = game.getPiece(bPos);

				REQUIRE(wQueen.type() == PieceTypes::QUEEN);
				REQUIRE(wQueen.player() == Players::WHITE);
				REQUIRE(bQueen.type() == PieceTypes::QUEEN);
				REQUIRE(bQueen.player() == Players::BLACK);
			} else if (file == Files::E) {
				Position wPos = {.file = file, .rank = 1};
				Position bPos = {.file = file, .rank = 8};

				REQUIRE(game.hasPiece(wPos));
				REQUIRE(game.hasPiece(bPos));

				Piece wKing = game.getPiece(wPos);
				Piece bKing = game.getPiece(bPos);

				REQUIRE(wKing.type() == PieceTypes::KING);
				REQUIRE(wKing.player() == Players::WHITE);
				REQUIRE(bKing.type() == PieceTypes::KING);
				REQUIRE(bKing.player() == Players::BLACK);
			}
		}
	}

	SECTION("Basic opening") {
		// King's pawn opening, black and white
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 2}, .to = {.file = Files::E, .rank = 4}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 7}, .to = {.file = Files::E, .rank = 5}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::G, .rank = 1}, .to = {.file = Files::F, .rank = 3}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::B, .rank = 8}, .to = {.file = Files::C, .rank = 6}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::F, .rank = 1}, .to = {.file = Files::B, .rank = 5}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::F, .rank = 8}, .to = {.file = Files::B, .rank = 4}}));

		REQUIRE_THROWS_WITH(game.move({.from = {.file = Files::D, .rank = 2}, .to = {.file = Files::D, .rank = 3}}),
							"Illegal move: moving into check/moving while in check.");
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::C, .rank = 2}, .to = {.file = Files::C, .rank = 3}}));
	}
}

TEST_CASE("Game check interactions") {
	Game game;

	SECTION("Moving out of check") {
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 2}, .to = {.file = Files::E, .rank = 4}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 7}, .to = {.file = Files::E, .rank = 5}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::G, .rank = 1}, .to = {.file = Files::F, .rank = 3}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::B, .rank = 8}, .to = {.file = Files::C, .rank = 6}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::D, .rank = 2}, .to = {.file = Files::D, .rank = 3}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::F, .rank = 8}, .to = {.file = Files::B, .rank = 4}}));

		REQUIRE_THROWS_WITH(game.move({.from = {.file = Files::F, .rank = 1}, .to = {.file = Files::E, .rank = 2}}),
							"Illegal move: moving into check/moving while in check.");
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 1}, .to = {.file = Files::E, .rank = 2}}));
	}

	SECTION("Blocking check") {
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 2}, .to = {.file = Files::E, .rank = 4}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 7}, .to = {.file = Files::E, .rank = 5}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::G, .rank = 1}, .to = {.file = Files::F, .rank = 3}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::B, .rank = 8}, .to = {.file = Files::C, .rank = 6}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::D, .rank = 2}, .to = {.file = Files::D, .rank = 3}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::F, .rank = 8}, .to = {.file = Files::B, .rank = 4}}));

		REQUIRE_THROWS_WITH(game.move({.from = {.file = Files::F, .rank = 1}, .to = {.file = Files::E, .rank = 2}}),
							"Illegal move: moving into check/moving while in check.");
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::C, .rank = 1}, .to = {.file = Files::D, .rank = 2}}));
	}
}

TEST_CASE("Game castling") {
	Game game;

	SECTION("Basic opening") {
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 2}, .to = {.file = Files::E, .rank = 4}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 7}, .to = {.file = Files::E, .rank = 5}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::G, .rank = 1}, .to = {.file = Files::F, .rank = 3}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::B, .rank = 8}, .to = {.file = Files::C, .rank = 6}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::F, .rank = 1}, .to = {.file = Files::B, .rank = 5}}));
		REQUIRE_NOTHROW(game.move({.from = {.file = Files::F, .rank = 8}, .to = {.file = Files::B, .rank = 4}}));

		REQUIRE_NOTHROW(game.move({.from = {.file = Files::E, .rank = 1}, .to = {.file = Files::G, .rank = 1}}));
	}
}