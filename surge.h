

#pragma warning(disable : 26812)

#include <cstdint>
#include <string>
#include <ostream>
#include <sstream>
#include <iostream>
#include <vector>

const size_t NCOLORS = 2;
enum Color : int {
	WHITE, BLACK
};

//Inverts the color (WHITE -> BLACK) and (BLACK -> WHITE)
constexpr Color operator~(Color c) {
	return Color(c ^ BLACK);
}

const size_t NDIRS = 8;
enum Direction : int {
	NORTH = 8, NORTH_EAST = 9, EAST = 1, SOUTH_EAST = -7,
	SOUTH = -8, SOUTH_WEST = -9, WEST = -1, NORTH_WEST = 7,
	NORTH_NORTH = 16, SOUTH_SOUTH = -16
};

const size_t NPIECE_TYPES = 6;
enum PieceType : int {
	PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING
};

//PIECE_STR[piece] is the algebraic chess representation of that piece
const std::string PIECE_STR = "PNBRQK~>pnbrqk.";

//The FEN of the starting position
const std::string DEFAULT_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";

//The Kiwipete position, used for perft debugging
const std::string KIWIPETE = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";

const size_t NPIECES = 15;
enum Piece : int {
	WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
	BLACK_PAWN = 8, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING,
	NO_PIECE
};

constexpr Piece make_piece(Color c, PieceType pt) {
	return Piece((c << 3) + pt);
}

constexpr PieceType type_of(Piece pc) {
	return PieceType(pc & 0b111);
}

constexpr Color color_of(Piece pc) {
	return Color((pc & 0b1000) >> 3);
}

typedef uint64_t Bitboard;

const size_t NSQUARES = 64;
enum Square : int {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8,
	NO_SQUARE
};

inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }
inline Square& operator-=(Square& s, Direction d) { return s = s - d; }

enum File : int {
	AFILE, BFILE, CFILE, DFILE, EFILE, FFILE, GFILE, HFILE
};	

enum Rank : int {
	RANK1, RANK2, RANK3, RANK4, RANK5, RANK6, RANK7, RANK8
};

extern const char* SQSTR[65];

extern const Bitboard MASK_FILE[8];
extern const Bitboard MASK_RANK[8];
extern const Bitboard MASK_DIAGONAL[15];
extern const Bitboard MASK_ANTI_DIAGONAL[15];
extern const Bitboard SQUARE_BB[65];

extern void print_bitboard(Bitboard b);

extern const Bitboard k1;
extern const Bitboard k2;
extern const Bitboard k4;
extern const Bitboard kf;

//gk extern inline int pop_count(Bitboard x);
extern int pop_count(Bitboard x);
//gk extern inline int sparse_pop_count(Bitboard x);
extern int sparse_pop_count(Bitboard x);
//gk extern inline Square pop_lsb(Bitboard* b);
extern Square pop_lsb(Bitboard* b);

extern const int DEBRUIJN64[64];
extern const Bitboard MAGIC;
//gk extern constexpr Square bsf(Bitboard b);
extern Square bsf(Bitboard b);

constexpr Rank rank_of(Square s) { return Rank(s >> 3); }
constexpr File file_of(Square s) { return File(s & 0b111); }
constexpr int diagonal_of(Square s) { return 7 + rank_of(s) - file_of(s); }
constexpr int anti_diagonal_of(Square s) { return rank_of(s) + file_of(s); }
constexpr Square create_square(File f, Rank r) { return Square(r << 3 | f); }

//Shifts a bitboard in a particular direction. There is no wrapping, so bits that are shifted of the edge are lost 
template<Direction D>
constexpr Bitboard shift(Bitboard b) {
	return D == NORTH ? b << 8 : D == SOUTH ? b >> 8
		: D == NORTH + NORTH ? b << 16 : D == SOUTH + SOUTH ? b >> 16
		: D == EAST ? (b & ~MASK_FILE[HFILE]) << 1 : D == WEST ? (b & ~MASK_FILE[AFILE]) >> 1
		: D == NORTH_EAST ? (b & ~MASK_FILE[HFILE]) << 9 
		: D == NORTH_WEST ? (b & ~MASK_FILE[AFILE]) << 7
		: D == SOUTH_EAST ? (b & ~MASK_FILE[HFILE]) >> 7 
		: D == SOUTH_WEST ? (b & ~MASK_FILE[AFILE]) >> 9
		: 0;	
}

//Returns the actual rank from a given side's perspective (e.g. rank 1 is rank 8 from Black's perspective)
template<Color C>
constexpr Rank relative_rank(Rank r) {
	return C == WHITE ? r : Rank(RANK8 - r);
}

//Returns the actual direction from a given side's perspective (e.g. North is South from Black's perspective)
template<Color C>
constexpr Direction relative_dir(Direction d) {
	return Direction(C == WHITE ? d : -d);
}

//The type of the move
enum MoveFlags : int {
	QUIET = 0b0000, DOUBLE_PUSH = 0b0001,
	OO = 0b0010, OOO = 0b0011,
	CAPTURE = 0b1000,
	CAPTURES = 0b1111,
	EN_PASSANT = 0b1010,
	PROMOTIONS = 0b0111,
	PROMOTION_CAPTURES = 0b1100,
	PR_KNIGHT = 0b0100, PR_BISHOP = 0b0101, PR_ROOK = 0b0110, PR_QUEEN = 0b0111,
	PC_KNIGHT = 0b1100, PC_BISHOP = 0b1101, PC_ROOK = 0b1110, PC_QUEEN = 0b1111,
};

class Move {
private:
	//The internal representation of the move
	uint16_t move;
public:
	//Defaults to a null move (a1a1)
	inline Move() : move(0) {}
	
	inline Move(uint16_t m) { move = m; }

	inline Move(Square from, Square to) : move(0) {
		move = (from << 6) | to;
	}

	inline Move(Square from, Square to, MoveFlags flags) : move(0) {
		move = (flags << 12) | (from << 6) | to;
	}

	Move(const std::string& move) {
		this->move = (create_square(File(move[0] - 'a'), Rank(move[1] - '1')) << 6) |
			create_square(File(move[2] - 'a'), Rank(move[3] - '1'));
	}

	inline Square to() const { return Square(move & 0x3f); }
	inline Square from() const { return Square((move >> 6) & 0x3f); }
	inline int to_from() const { return move & 0xffff; }
	inline MoveFlags flags() const { return MoveFlags((move >> 12) & 0xf); }

	inline bool is_capture() const {
		return (move >> 12) & CAPTURES;
	}

	void operator=(Move m) { move = m.move; }
	bool operator==(Move a) const { return to_from() == a.to_from(); }
	bool operator!=(Move a) const { return to_from() != a.to_from(); }
};

//Adds, to the move pointer all moves of the form (from, s), where s is a square in the bitboard to
template<MoveFlags F = QUIET>
inline Move *make(Square from, Bitboard to, Move *list) {
	while (to) *list++ = Move(from, pop_lsb(&to), F);
	return list;
}

//Adds, to the move pointer all quiet promotion moves of the form (from, s), where s is a square in the bitboard to
template<>
inline Move *make<PROMOTIONS>(Square from, Bitboard to, Move *list) {
	Square p;
	while (to) {
		p = pop_lsb(&to);
		*list++ = Move(from, p, PR_KNIGHT);
		*list++ = Move(from, p, PR_BISHOP);
		*list++ = Move(from, p, PR_ROOK);
		*list++ = Move(from, p, PR_QUEEN);
	}
	return list;
}

//Adds, to the move pointer all capture promotion moves of the form (from, s), where s is a square in the bitboard to
template<>
inline Move* make<PROMOTION_CAPTURES>(Square from, Bitboard to, Move* list) {
	Square p;
	while (to) {
		p = pop_lsb(&to);
		*list++ = Move(from, p, PC_KNIGHT);
		*list++ = Move(from, p, PC_BISHOP);
		*list++ = Move(from, p, PC_ROOK);
		*list++ = Move(from, p, PC_QUEEN);
	}
	return list;
}

extern std::ostream& operator<<(std::ostream& os, const Move& m);

//The white king and kingside rook
const Bitboard WHITE_OO_MASK = 0x90;
//The white king and queenside rook
const Bitboard WHITE_OOO_MASK = 0x11;

//Squares between the white king and kingside rook
const Bitboard WHITE_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x60;
//Squares between the white king and queenside rook
const Bitboard WHITE_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xe;

//The black king and kingside rook
const Bitboard BLACK_OO_MASK = 0x9000000000000000;
//The black king and queenside rook
const Bitboard BLACK_OOO_MASK = 0x1100000000000000;
//Squares between the black king and kingside rook
const Bitboard BLACK_OO_BLOCKERS_AND_ATTACKERS_MASK = 0x6000000000000000;
//Squares between the black king and queenside rook
const Bitboard BLACK_OOO_BLOCKERS_AND_ATTACKERS_MASK = 0xE00000000000000;

//The white king, white rooks, black king and black rooks
const Bitboard ALL_CASTLING_MASK = 0x9100000000000091;

template<Color C> constexpr Bitboard oo_mask() { return C == WHITE ? WHITE_OO_MASK : BLACK_OO_MASK; }
template<Color C> constexpr Bitboard ooo_mask() { return C == WHITE ? WHITE_OOO_MASK : BLACK_OOO_MASK; }

template<Color C>
constexpr Bitboard oo_blockers_mask() { 
	return C == WHITE ? WHITE_OO_BLOCKERS_AND_ATTACKERS_MASK :
		BLACK_OO_BLOCKERS_AND_ATTACKERS_MASK; 
}

template<Color C>
constexpr Bitboard ooo_blockers_mask() {
	return C == WHITE ? WHITE_OOO_BLOCKERS_AND_ATTACKERS_MASK :
		BLACK_OOO_BLOCKERS_AND_ATTACKERS_MASK;
}
	
template<Color C> constexpr Bitboard ignore_ooo_danger() { return C == WHITE ? 0x2 : 0x200000000000000; }

#include <ostream>
#include <string>

extern const Bitboard KING_ATTACKS[NSQUARES];
extern const Bitboard KNIGHT_ATTACKS[NSQUARES];
extern const Bitboard WHITE_PAWN_ATTACKS[NSQUARES];
extern const Bitboard BLACK_PAWN_ATTACKS[NSQUARES];

extern Bitboard reverse(Bitboard b);
extern Bitboard sliding_attacks(Square square, Bitboard occ, Bitboard mask);

extern Bitboard get_rook_attacks_for_init(Square square, Bitboard occ);
extern const Bitboard ROOK_MAGICS[NSQUARES];
extern Bitboard ROOK_ATTACK_MASKS[NSQUARES];
extern int ROOK_ATTACK_SHIFTS[NSQUARES];
extern Bitboard ROOK_ATTACKS[NSQUARES][4096];
extern void initialise_rook_attacks();

//gk extern constexpr Bitboard get_rook_attacks(Square square, Bitboard occ);
extern Bitboard get_rook_attacks(Square square, Bitboard occ);
extern Bitboard get_xray_rook_attacks(Square square, Bitboard occ, Bitboard blockers);

extern Bitboard get_bishop_attacks_for_init(Square square, Bitboard occ);
extern const Bitboard BISHOP_MAGICS[NSQUARES];
extern Bitboard BISHOP_ATTACK_MASKS[NSQUARES];
extern int BISHOP_ATTACK_SHIFTS[NSQUARES];
extern Bitboard BISHOP_ATTACKS[NSQUARES][512];
extern void initialise_bishop_attacks();

//gk extern constexpr Bitboard get_bishop_attacks(Square square, Bitboard occ);
extern Bitboard get_bishop_attacks(Square square, Bitboard occ);
extern Bitboard get_xray_bishop_attacks(Square square, Bitboard occ, Bitboard blockers);

extern Bitboard SQUARES_BETWEEN_BB[NSQUARES][NSQUARES];
extern Bitboard LINE[NSQUARES][NSQUARES];
extern Bitboard PAWN_ATTACKS[NCOLORS][NSQUARES];
extern Bitboard PSEUDO_LEGAL_ATTACKS[NPIECE_TYPES][NSQUARES];

extern void initialise_squares_between();
extern void initialise_line();
extern void initialise_pseudo_legal();
extern void initialise_all_databases();

//Returns a bitboard containing all squares that a piece on a square can move to, in the given position
template<PieceType P>
constexpr Bitboard attacks(Square s, Bitboard occ) {
	static_assert(P != PAWN, "The piece type may not be a pawn; use pawn_attacks instead");
	return P == ROOK ? get_rook_attacks(s, occ) :
		P == BISHOP ? get_bishop_attacks(s, occ) :
		P == QUEEN ? attacks<ROOK>(s, occ) | attacks<BISHOP>(s, occ) :
		PSEUDO_LEGAL_ATTACKS[P][s];
}

//Returns a bitboard containing all squares that a piece on a square can move to, in the given position
//Used when the piece type is not known at compile-time
//gk (needs at least c++14)
constexpr Bitboard attacks(PieceType pt, Square s, Bitboard occ) {
	switch (pt) {
	case ROOK:
		return attacks<ROOK>(s, occ);
	case BISHOP:
		return attacks<BISHOP>(s, occ);
	case QUEEN:
		return attacks<QUEEN>(s, occ);
	default:
		return PSEUDO_LEGAL_ATTACKS[pt][s];
	}
}

//Returns a bitboard containing pawn attacks from all pawns in the given bitboard
template<Color C>
constexpr Bitboard pawn_attacks(Bitboard p) {
	return C == WHITE ? shift<NORTH_WEST>(p) | shift<NORTH_EAST>(p) :
		shift<SOUTH_WEST>(p) | shift<SOUTH_EAST>(p);
}

//Returns a bitboard containing pawn attacks from the pawn on the given square
template<Color C>
constexpr Bitboard pawn_attacks(Square s) {
	return PAWN_ATTACKS[C][s];
}

#include <utility>

//A psuedorandom number generator
//Source: Stockfish
class PRNG {
	uint64_t s;

	uint64_t rand64() {
		s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
		return s * 2685821657736338717LL;
	}

public:
	PRNG(uint64_t seed) : s(seed) {}

	//Generate psuedorandom number
	template<typename T> T rand() { return T(rand64()); }

	//Generate psuedorandom number with only a few set bits
	template<typename T> 
	T sparse_rand() {
		return T(rand64() & rand64() & rand64());
	}
};

namespace zobrist {
	extern uint64_t zobrist_table[NPIECES][NSQUARES];
	extern void initialise_zobrist_keys();
}

//Stores position information which cannot be recovered on undo-ing a move
struct UndoInfo {
	//The bitboard of squares on which pieces have either moved from, or have been moved to. Used for castling
	//legality checks
	Bitboard entry;
	
	//The piece that was captured on the last move
	Piece captured;
	
	//The en passant square. This is the square which pawns can move to in order to en passant capture an enemy pawn that has 
	//double pushed on the previous move
	Square epsq;

	constexpr UndoInfo() : entry(0), captured(NO_PIECE), epsq(NO_SQUARE) {}
	
	//This preserves the entry bitboard across moves
	UndoInfo(const UndoInfo& prev) : 
		entry(prev.entry), captured(NO_PIECE), epsq(NO_SQUARE) {}
};

class Position {
private:
	//A bitboard of the locations of each piece
	Bitboard piece_bb[NPIECES];
	
	//A mailbox representation of the board. Stores the piece occupying each square on the board
	Piece board[NSQUARES];
	
	//The side whose turn it is to play next
	Color side_to_play;
	
	//The current game ply (depth), incremented after each move 
	int game_ply;
	
	//The zobrist hash of the position, which can be incrementally updated and rolled back after each
	//make/unmake
	uint64_t hash;
public:
	//The history of non-recoverable information
	UndoInfo history[256];
	
	//The bitboard of enemy pieces that are currently attacking the king, updated whenever generate_moves()
	//is called
	Bitboard checkers;
	
	//The bitboard of pieces that are currently pinned to the king by enemy sliders, updated whenever 
	//generate_moves() is called
	Bitboard pinned;
	
	
//gk adapted order of initialization
//gk	Position() : piece_bb{ 0 }, side_to_play(WHITE), game_ply(0), board{}, 
//gk		hash(0), pinned(0), checkers(0) {
	Position() : piece_bb{ 0 }, board{}, side_to_play(WHITE), game_ply(0),
		hash(0), checkers(0), pinned(0) {
		
		//Sets all squares on the board as empty
		for (int i = 0; i < 64; i++) board[i] = NO_PIECE;
		history[0] = UndoInfo();
	}
  
	Position(const std::string& fen): Position(){
    int square = a8;
    for (char ch : fen.substr(0, fen.find(' '))) {
      if (isdigit(ch))
        square += (ch - '0') * EAST;
      else if (ch == '/')
        square += 2 * SOUTH;
      else
        put_piece(Piece(PIECE_STR.find(ch)), Square(square++));
    }

    std::istringstream ss(fen.substr(fen.find(' ')));
    unsigned char token;

    ss >> token;
    side_to_play = token == 'w' ? WHITE : BLACK;

    history[game_ply].entry = ALL_CASTLING_MASK;
    while (ss >> token && !isspace(token)) {
      switch (token) {
      case 'K':
        history[game_ply].entry &= ~WHITE_OO_MASK;
        break;
      case 'Q':
        history[game_ply].entry &= ~WHITE_OOO_MASK;
        break;
      case 'k':
        history[game_ply].entry &= ~BLACK_OO_MASK;
        break;
      case 'q':
        history[game_ply].entry &= ~BLACK_OOO_MASK;
        break;
      }
    }
  }
	//Places a piece on a particular square and updates the hash. Placing a piece on a square that is 
	//already occupied is an error
	inline void put_piece(Piece pc, Square s) {
		board[s] = pc;
		piece_bb[pc] |= SQUARE_BB[s];
		hash ^= zobrist::zobrist_table[pc][s];
	}

	//Removes a piece from a particular square and updates the hash. 
	inline void remove_piece(Square s) {
		hash ^= zobrist::zobrist_table[board[s]][s];
		piece_bb[board[s]] &= ~SQUARE_BB[s];
		board[s] = NO_PIECE;
	}

	void move_piece(Square from, Square to);
	void move_piece_quiet(Square from, Square to);

	friend std::ostream& operator<<(std::ostream& os, const Position& p);
	std::string fen() const;

	inline bool operator==(const Position& other) const { return hash == other.hash; }

	inline Bitboard bitboard_of(Piece pc) const { return piece_bb[pc]; }
	inline Bitboard bitboard_of(Color c, PieceType pt) const { return piece_bb[make_piece(c, pt)]; }
	inline Piece at(Square sq) const { return board[sq]; }
	inline Color turn() const { return side_to_play; }
	inline int ply() const { return game_ply; }
	inline uint64_t get_hash() const { return hash; }

	template<Color C> inline Bitboard diagonal_sliders() const;
	template<Color C> inline Bitboard orthogonal_sliders() const;
	template<Color C> inline Bitboard all_pieces() const;
	template<Color C> inline Bitboard attackers_from(Square s, Bitboard occ) const;

	template<Color C> inline bool in_check() const {
		return attackers_from<~C>(bsf(bitboard_of(C, KING)), all_pieces<WHITE>() | all_pieces<BLACK>());
	}

	template<Color C> void play(Move m);
	template<Color C> void undo(Move m);

	template<Color Us>
	Move *generate_legals(Move* list);
};

//Returns the bitboard of all bishops and queens of a given color
template<Color C> 
inline Bitboard Position::diagonal_sliders() const {
	return C == WHITE ? piece_bb[WHITE_BISHOP] | piece_bb[WHITE_QUEEN] :
		piece_bb[BLACK_BISHOP] | piece_bb[BLACK_QUEEN];
}

//Returns the bitboard of all rooks and queens of a given color
template<Color C> 
inline Bitboard Position::orthogonal_sliders() const {
	return C == WHITE ? piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN] :
		piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN];
}

//Returns a bitboard containing all the pieces of a given color
template<Color C> 
inline Bitboard Position::all_pieces() const {
	return C == WHITE ? piece_bb[WHITE_PAWN] | piece_bb[WHITE_KNIGHT] | piece_bb[WHITE_BISHOP] |
		piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN] | piece_bb[WHITE_KING] :

		piece_bb[BLACK_PAWN] | piece_bb[BLACK_KNIGHT] | piece_bb[BLACK_BISHOP] |
		piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN] | piece_bb[BLACK_KING];
}

//Returns a bitboard containing all pieces of a given color attacking a particluar square
template<Color C> 
inline Bitboard Position::attackers_from(Square s, Bitboard occ) const {
	return C == WHITE ? (pawn_attacks<BLACK>(s) & piece_bb[WHITE_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[WHITE_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[WHITE_BISHOP] | piece_bb[WHITE_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[WHITE_ROOK] | piece_bb[WHITE_QUEEN])) :

		(pawn_attacks<WHITE>(s) & piece_bb[BLACK_PAWN]) |
		(attacks<KNIGHT>(s, occ) & piece_bb[BLACK_KNIGHT]) |
		(attacks<BISHOP>(s, occ) & (piece_bb[BLACK_BISHOP] | piece_bb[BLACK_QUEEN])) |
		(attacks<ROOK>(s, occ) & (piece_bb[BLACK_ROOK] | piece_bb[BLACK_QUEEN]));
}

/*template<Color C>
Bitboard Position::pinned(Square s, Bitboard us, Bitboard occ) const {
	Bitboard pinned = 0;

	Bitboard pinners = get_xray_rook_attacks(s, occ, us) & orthogonal_sliders<~C>();
	while (pinners) pinned |= SQUARES_BETWEEN_BB[s][pop_lsb(&pinners)] & us;

	pinners = get_xray_bishop_attacks(s, occ, us) & diagonal_sliders<~C>();
	while (pinners) pinned |= SQUARES_BETWEEN_BB[s][pop_lsb(&pinners)] & us;

	return pinned;
}

template<Color C>
Bitboard Position::blockers_to(Square s, Bitboard occ) const {
	Bitboard blockers = 0;
	Bitboard candidates = get_rook_attacks(s, occ) & occ;
	Bitboard attackers = get_rook_attacks(s, occ ^ candidates) & orthogonal_sliders<~C>();

	candidates = get_bishop_attacks(s, occ) & occ;
	attackers |= get_bishop_attacks(s, occ ^ candidates) & diagonal_sliders<~C>();

	while (attackers) blockers |= SQUARES_BETWEEN_BB[s][pop_lsb(&attackers)];
	return blockers;
}*/

//Plays a move in the position
template<Color C>
void Position::play(const Move m) {
	side_to_play = ~side_to_play;
	++game_ply;
	history[game_ply] = UndoInfo(history[game_ply - 1]);

	MoveFlags type = m.flags();
	history[game_ply].entry |= SQUARE_BB[m.to()] | SQUARE_BB[m.from()];

	switch (type) {
	case QUIET:
		//The to square is guaranteed to be empty here
		move_piece_quiet(m.from(), m.to());
		break;
	case DOUBLE_PUSH:
		//The to square is guaranteed to be empty here
		move_piece_quiet(m.from(), m.to());
			
		//This is the square behind the pawn that was double-pushed
		history[game_ply].epsq = m.from() + relative_dir<C>(NORTH);
		break;
	case OO:
		if (C == WHITE) {
			move_piece_quiet(e1, g1);
			move_piece_quiet(h1, f1);
		} else {
			move_piece_quiet(e8, g8);
			move_piece_quiet(h8, f8);
		}			
		break;
	case OOO:
		if (C == WHITE) {
			move_piece_quiet(e1, c1); 
			move_piece_quiet(a1, d1);
		} else {
			move_piece_quiet(e8, c8);
			move_piece_quiet(a8, d8);
		}
		break;
	case EN_PASSANT:
		move_piece_quiet(m.from(), m.to());
		remove_piece(m.to() + relative_dir<C>(SOUTH));
		break;
	case PR_KNIGHT:
		remove_piece(m.from());
		put_piece(make_piece(C, KNIGHT), m.to());
		break;
	case PR_BISHOP:
		remove_piece(m.from());
		put_piece(make_piece(C, BISHOP), m.to());
		break;
	case PR_ROOK:
		remove_piece(m.from());
		put_piece(make_piece(C, ROOK), m.to());
		break;
	case PR_QUEEN:
		remove_piece(m.from());
		put_piece(make_piece(C, QUEEN), m.to());
		break;
	case PC_KNIGHT:
		remove_piece(m.from());
		history[game_ply].captured = board[m.to()];
		remove_piece(m.to());
		
		put_piece(make_piece(C, KNIGHT), m.to());
		break;
	case PC_BISHOP:
		remove_piece(m.from());
		history[game_ply].captured = board[m.to()];
		remove_piece(m.to());

		put_piece(make_piece(C, BISHOP), m.to());
		break;
	case PC_ROOK:
		remove_piece(m.from());
		history[game_ply].captured = board[m.to()];
		remove_piece(m.to());

		put_piece(make_piece(C, ROOK), m.to());
		break;
	case PC_QUEEN:
		remove_piece(m.from());
		history[game_ply].captured = board[m.to()];
		remove_piece(m.to());

		put_piece(make_piece(C, QUEEN), m.to());
		break;
	case CAPTURE:
		history[game_ply].captured = board[m.to()];
		move_piece(m.from(), m.to());
		
		break;
	}
}

//Undos a move in the current position, rolling it back to the previous position
template<Color C>
void Position::undo(const Move m) {
	MoveFlags type = m.flags();
	switch (type) {
	case QUIET:
		move_piece_quiet(m.to(), m.from());
		break;
	case DOUBLE_PUSH:
		move_piece_quiet(m.to(), m.from());
		break;
	case OO:
		if (C == WHITE) {
			move_piece_quiet(g1, e1);
			move_piece_quiet(f1, h1);
		} else {
			move_piece_quiet(g8, e8);
			move_piece_quiet(f8, h8);
		}
		break;
	case OOO:
		if (C == WHITE) {
			move_piece_quiet(c1, e1);
			move_piece_quiet(d1, a1);
		} else {
			move_piece_quiet(c8, e8);
			move_piece_quiet(d8, a8);
		}
		break;
	case EN_PASSANT:
		move_piece_quiet(m.to(), m.from());
		put_piece(make_piece(~C, PAWN), m.to() + relative_dir<C>(SOUTH));
		break;
	case PR_KNIGHT:
	case PR_BISHOP:
	case PR_ROOK:
	case PR_QUEEN:
		remove_piece(m.to());
		put_piece(make_piece(C, PAWN), m.from());
		break;
	case PC_KNIGHT:
	case PC_BISHOP:
	case PC_ROOK:
	case PC_QUEEN:
		remove_piece(m.to());
		put_piece(make_piece(C, PAWN), m.from());
		put_piece(history[game_ply].captured, m.to());
		break;
	case CAPTURE:
		move_piece_quiet(m.to(), m.from());
		put_piece(history[game_ply].captured, m.to());
		break;
	}

	side_to_play = ~side_to_play;
	--game_ply;
}

//Generates all legal moves in a position for the given side. Advances the move pointer and returns it.
template<Color Us>
Move* Position::generate_legals(Move* list) {
	constexpr Color Them = ~Us;

	const Bitboard us_bb = all_pieces<Us>();
	const Bitboard them_bb = all_pieces<Them>();
	const Bitboard all = us_bb | them_bb;

	const Square our_king = bsf(bitboard_of(Us, KING));
	const Square their_king = bsf(bitboard_of(Them, KING));

	const Bitboard our_diag_sliders = diagonal_sliders<Us>();
	const Bitboard their_diag_sliders = diagonal_sliders<Them>();
	const Bitboard our_orth_sliders = orthogonal_sliders<Us>();
	const Bitboard their_orth_sliders = orthogonal_sliders<Them>();

	//General purpose bitboards for attacks, masks, etc.
	Bitboard b1, b2, b3;
	
	//Squares that our king cannot move to
	Bitboard danger = 0;

	//For each enemy piece, add all of its attacks to the danger bitboard
	danger |= pawn_attacks<Them>(bitboard_of(Them, PAWN)) | attacks<KING>(their_king, all);
	
	b1 = bitboard_of(Them, KNIGHT); 
	while (b1) danger |= attacks<KNIGHT>(pop_lsb(&b1), all);
	
	b1 = their_diag_sliders;
	//all ^ SQUARE_BB[our_king] is written to prevent the king from moving to squares which are 'x-rayed'
	//by enemy bishops and queens
	while (b1) danger |= attacks<BISHOP>(pop_lsb(&b1), all ^ SQUARE_BB[our_king]);
	
	b1 = their_orth_sliders;
	//all ^ SQUARE_BB[our_king] is written to prevent the king from moving to squares which are 'x-rayed'
	//by enemy rooks and queens
	while (b1) danger |= attacks<ROOK>(pop_lsb(&b1), all ^ SQUARE_BB[our_king]);

	//The king can move to all of its surrounding squares, except ones that are attacked, and
	//ones that have our own pieces on them
	b1 = attacks<KING>(our_king, all) & ~(us_bb | danger);
	list = make<QUIET>(our_king, b1 & ~them_bb, list);
	list = make<CAPTURE>(our_king, b1 & them_bb, list);

	//The capture mask filters destination squares to those that contain an enemy piece that is checking the 
	//king and must be captured
	Bitboard capture_mask;
	
	//The quiet mask filter destination squares to those where pieces must be moved to block an incoming attack 
	//to the king
	Bitboard quiet_mask;
	
	//A general purpose square for storing destinations, etc.
	Square s;

	//Checkers of each piece type are identified by:
	//1. Projecting attacks FROM the king square
	//2. Intersecting this bitboard with the enemy bitboard of that piece type
	//gk additional parentheses
	checkers = (attacks<KNIGHT>(our_king, all) & bitboard_of(Them, KNIGHT))
		| (pawn_attacks<Us>(our_king) & bitboard_of(Them, PAWN));
	
	//Here, we identify slider checkers and pinners simultaneously, and candidates for such pinners 
	//and checkers are represented by the bitboard <candidates>
	//gk additional parentheses
	Bitboard candidates = (attacks<ROOK>(our_king, them_bb) & their_orth_sliders)
		| (attacks<BISHOP>(our_king, them_bb) & their_diag_sliders);

	pinned = 0;
	while (candidates) {
		s = pop_lsb(&candidates);
		b1 = SQUARES_BETWEEN_BB[our_king][s] & us_bb;
		
		//Do the squares in between the enemy slider and our king contain any of our pieces?
		//If not, add the slider to the checker bitboard
		if (b1 == 0) checkers ^= SQUARE_BB[s];
		//If there is only one of our pieces between them, add our piece to the pinned bitboard 
		//gk additional parentheses
		else if ((b1 & (b1 - 1)) == 0) pinned ^= b1;
	}

	//This makes it easier to mask pieces
	const Bitboard not_pinned = ~pinned;

	switch (sparse_pop_count(checkers)) {
	case 2:
		//If there is a double check, the only legal moves are king moves out of check
		return list;
	case 1: {
		//It's a single check!
		
		Square checker_square = bsf(checkers);

		switch (board[checker_square]) {
		case make_piece(Them, PAWN):
			//If the checker is a pawn, we must check for e.p. moves that can capture it
			//This evaluates to true if the checking piece is the one which just double pushed
			if (checkers == shift<relative_dir<Us>(SOUTH)>(SQUARE_BB[history[game_ply].epsq])) {
				//b1 contains our pawns that can capture the checker e.p.
				b1 = pawn_attacks<Them>(history[game_ply].epsq) & bitboard_of(Us, PAWN) & not_pinned;
				while (b1) *list++ = Move(pop_lsb(&b1), history[game_ply].epsq, EN_PASSANT);
			}
			//FALL THROUGH INTENTIONAL
		case make_piece(Them, KNIGHT):
			//If the checker is either a pawn or a knight, the only legal moves are to capture
			//the checker. Only non-pinned pieces can capture it
			b1 = attackers_from<Us>(checker_square, all) & not_pinned;
			while (b1) {
				s = pop_lsb(&b1);
				if (type_of(board[s]) == PAWN && rank_of(s) == relative_rank<Us>(RANK7)) {
					*list++ = Move(s, checker_square, PC_QUEEN);
					*list++ = Move(s, checker_square, PC_ROOK);
					*list++ = Move(s, checker_square, PC_BISHOP);
					*list++ = Move(s, checker_square, PC_KNIGHT);
				}
				else *list++ = Move(s, checker_square, CAPTURE);
			}

			return list;
		default:
			//We must capture the checking piece
			capture_mask = checkers;
			
			//...or we can block it since it is guaranteed to be a slider
			quiet_mask = SQUARES_BETWEEN_BB[our_king][checker_square];
			break;
		}

		break;
	}

	default:
		//We can capture any enemy piece
		capture_mask = them_bb;
		
		//...and we can play a quiet move to any square which is not occupied
		quiet_mask = ~all;

		if (history[game_ply].epsq != NO_SQUARE) {
			//b1 contains our pawns that can perform an e.p. capture
			b2 = pawn_attacks<Them>(history[game_ply].epsq) & bitboard_of(Us, PAWN);
			b1 = b2 & not_pinned;
			while (b1) {
				s = pop_lsb(&b1);
				
				//This piece of evil bit-fiddling magic prevents the infamous 'pseudo-pinned' e.p. case,
				//where the pawn is not directly pinned, but on moving the pawn and capturing the enemy pawn
				//e.p., a rook or queen attack to the king is revealed
				
				/*
				.nbqkbnr
				ppp.pppp
				........
				r..pP..K
				........
				........
				PPPP.PPP
				RNBQ.BNR
				
				Here, if white plays exd5 e.p., the black rook on a5 attacks the white king on h5 
				*/
				
				if ((sliding_attacks(our_king, all ^ SQUARE_BB[s]
					^ shift<relative_dir<Us>(SOUTH)>(SQUARE_BB[history[game_ply].epsq]),
					MASK_RANK[rank_of(our_king)]) &
					their_orth_sliders) == 0)
						*list++ = Move(s, history[game_ply].epsq, EN_PASSANT);
			}
			
			//Pinned pawns can only capture e.p. if they are pinned diagonally and the e.p. square is in line with the king 
			b1 = b2 & pinned & LINE[history[game_ply].epsq][our_king];
			if (b1) {
				*list++ = Move(bsf(b1), history[game_ply].epsq, EN_PASSANT);
			}
		}

		//Only add castling if:
		//1. The king and the rook have both not moved
		//2. No piece is attacking between the the rook and the king
		//3. The king is not in check
		if (!((history[game_ply].entry & oo_mask<Us>()) | ((all | danger) & oo_blockers_mask<Us>())))
			*list++ = Us == WHITE ? Move(e1, h1, OO) : Move(e8, h8, OO);
		if (!((history[game_ply].entry & ooo_mask<Us>()) |
			((all | (danger & ~ignore_ooo_danger<Us>())) & ooo_blockers_mask<Us>())))
			*list++ = Us == WHITE ? Move(e1, c1, OOO) : Move(e8, c8, OOO);

		//For each pinned rook, bishop or queen...
		b1 = ~(not_pinned | bitboard_of(Us, KNIGHT));
		while (b1) {
			s = pop_lsb(&b1);
			
			//...only include attacks that are aligned with our king, since pinned pieces
			//are constrained to move in this direction only
			b2 = attacks(type_of(board[s]), s, all) & LINE[our_king][s];
			list = make<QUIET>(s, b2 & quiet_mask, list);
			list = make<CAPTURE>(s, b2 & capture_mask, list);
		}

		//For each pinned pawn...
		b1 = ~not_pinned & bitboard_of(Us, PAWN);
		while (b1) {
			s = pop_lsb(&b1);

			if (rank_of(s) == relative_rank<Us>(RANK7)) {
				//Quiet promotions are impossible since the square in front of the pawn will
				//either be occupied by the king or the pinner, or doing so would leave our king
				//in check
				b2 = pawn_attacks<Us>(s) & capture_mask & LINE[our_king][s];
				list = make<PROMOTION_CAPTURES>(s, b2, list);
			}
			else {
				b2 = pawn_attacks<Us>(s) & them_bb & LINE[s][our_king];
				list = make<CAPTURE>(s, b2, list);
				
				//Single pawn pushes
				b2 = shift<relative_dir<Us>(NORTH)>(SQUARE_BB[s]) & ~all & LINE[our_king][s];
				//Double pawn pushes (only pawns on rank 3/6 are eligible)
				b3 = shift<relative_dir<Us>(NORTH)>(b2 &
					MASK_RANK[relative_rank<Us>(RANK3)]) & ~all & LINE[our_king][s];
				list = make<QUIET>(s, b2, list);
				list = make<DOUBLE_PUSH>(s, b3, list);
			}
		}
		
		//Pinned knights cannot move anywhere, so we're done with pinned pieces!

		break;
	}

	//Non-pinned knight moves
	b1 = bitboard_of(Us, KNIGHT) & not_pinned;
	while (b1) {
		s = pop_lsb(&b1);
		b2 = attacks<KNIGHT>(s, all);
		list = make<QUIET>(s, b2 & quiet_mask, list);
		list = make<CAPTURE>(s, b2 & capture_mask, list);
	}

	//Non-pinned bishops and queens
	b1 = our_diag_sliders & not_pinned;
	while (b1) {
		s = pop_lsb(&b1);
		b2 = attacks<BISHOP>(s, all);
		list = make<QUIET>(s, b2 & quiet_mask, list);
		list = make<CAPTURE>(s, b2 & capture_mask, list);
	}

	//Non-pinned rooks and queens
	b1 = our_orth_sliders & not_pinned;
	while (b1) {
		s = pop_lsb(&b1);
		b2 = attacks<ROOK>(s, all);
		list = make<QUIET>(s, b2 & quiet_mask, list);
		list = make<CAPTURE>(s, b2 & capture_mask, list);
	}

	//b1 contains non-pinned pawns which are not on the last rank
	b1 = bitboard_of(Us, PAWN) & not_pinned & ~MASK_RANK[relative_rank<Us>(RANK7)];
	
	//Single pawn pushes
	b2 = shift<relative_dir<Us>(NORTH)>(b1) & ~all;
	
	//Double pawn pushes (only pawns on rank 3/6 are eligible)
	b3 = shift<relative_dir<Us>(NORTH)>(b2 & MASK_RANK[relative_rank<Us>(RANK3)]) & quiet_mask;
	
	//We & this with the quiet mask only later, as a non-check-blocking single push does NOT mean that the 
	//corresponding double push is not blocking check either.
	b2 &= quiet_mask;

	while (b2) {
		s = pop_lsb(&b2);
		*list++ = Move(s - relative_dir<Us>(NORTH), s, QUIET);
	}

	while (b3) {
		s = pop_lsb(&b3);
		*list++ = Move(s - relative_dir<Us>(NORTH_NORTH), s, DOUBLE_PUSH);
	}

	//Pawn captures
	b2 = shift<relative_dir<Us>(NORTH_WEST)>(b1) & capture_mask;
	b3 = shift<relative_dir<Us>(NORTH_EAST)>(b1) & capture_mask;

	while (b2) {
		s = pop_lsb(&b2);
		*list++ = Move(s - relative_dir<Us>(NORTH_WEST), s, CAPTURE);
	}

	while (b3) {
		s = pop_lsb(&b3);
		*list++ = Move(s - relative_dir<Us>(NORTH_EAST), s, CAPTURE);
	}

	//b1 now contains non-pinned pawns which ARE on the last rank (about to promote)
	b1 = bitboard_of(Us, PAWN) & not_pinned & MASK_RANK[relative_rank<Us>(RANK7)];
	if (b1) {
		//Quiet promotions
		b2 = shift<relative_dir<Us>(NORTH)>(b1) & quiet_mask;
		while (b2) {
			s = pop_lsb(&b2);
			//One move is added for each promotion piece
			*list++ = Move(s - relative_dir<Us>(NORTH), s, PR_KNIGHT);
			*list++ = Move(s - relative_dir<Us>(NORTH), s, PR_BISHOP);
			*list++ = Move(s - relative_dir<Us>(NORTH), s, PR_ROOK);
			*list++ = Move(s - relative_dir<Us>(NORTH), s, PR_QUEEN);
		}

		//Promotion captures
		b2 = shift<relative_dir<Us>(NORTH_WEST)>(b1) & capture_mask;
		b3 = shift<relative_dir<Us>(NORTH_EAST)>(b1) & capture_mask;

		while (b2) {
			s = pop_lsb(&b2);
			//One move is added for each promotion piece
			*list++ = Move(s - relative_dir<Us>(NORTH_WEST), s, PC_KNIGHT);
			*list++ = Move(s - relative_dir<Us>(NORTH_WEST), s, PC_BISHOP);
			*list++ = Move(s - relative_dir<Us>(NORTH_WEST), s, PC_ROOK);
			*list++ = Move(s - relative_dir<Us>(NORTH_WEST), s, PC_QUEEN);
		}

		while (b3) {
			s = pop_lsb(&b3);
			//One move is added for each promotion piece
			*list++ = Move(s - relative_dir<Us>(NORTH_EAST), s, PC_KNIGHT);
			*list++ = Move(s - relative_dir<Us>(NORTH_EAST), s, PC_BISHOP);
			*list++ = Move(s - relative_dir<Us>(NORTH_EAST), s, PC_ROOK);
			*list++ = Move(s - relative_dir<Us>(NORTH_EAST), s, PC_QUEEN);
		}
	}

	return list;
}

//A convenience class for interfacing with legal moves, rather than using the low-level
//generate_legals() function directly. It can be iterated over.
template<Color Us>
class MoveList {
public:
	explicit MoveList(Position& p) : last(p.generate_legals<Us>(list)) {}

	const Move* begin() const { return list; }
	const Move* end() const { return last; }
	size_t size() const { return last - list; }
private:
	Move list[218];
	Move *last;
};

//Zobrist keys for each piece and each square
//Used to incrementally update the hash key of a position
uint64_t zobrist::zobrist_table[NPIECES][NSQUARES];

//Initializes the zobrist table with random 64-bit numbers
void zobrist::initialise_zobrist_keys() {
	PRNG rng(70026072);
	//gk comparison of integer expressions of different signedness
	//gk for (int i = 0; i < NPIECES; i++)
	//gk    for (int j = 0; j < NSQUARES; j++)
	for (size_t i = 0; i < NPIECES; i++)
		for (size_t j = 0; j < NSQUARES; j++)
			zobrist::zobrist_table[i][j] = rng.rand<uint64_t>();
}

//Pretty-prints the position (including FEN and hash key)
std::ostream& operator<< (std::ostream& os, const Position& p) {
	const char* s = "   +---+---+---+---+---+---+---+---+\n";
	const char* t = "     A   B   C   D   E   F   G   H\n";
	os << t;
	for (int i = 56; i >= 0; i -= 8) {
		os << s << " " << i / 8 + 1 << " ";
		for (int j = 0; j < 8; j++)
			os << "| " << PIECE_STR[p.board[i + j]] << " ";
		os << "| " << i / 8 + 1 << "\n";
	}
	os << s;
	os << t << "\n";

	os << "FEN: " << p.fen() << "\n";
	os << "Hash: 0x" << std::hex << p.hash << std::dec << "\n";

	return os;
}

//Returns the FEN (Forsyth-Edwards Notation) representation of the position
std::string Position::fen() const {
	std::ostringstream fen;
	int empty;

	for (int i = 56; i >= 0; i -= 8) {
		empty = 0;
		for (int j = 0; j < 8; j++) {
			Piece p = board[i + j];
			if (p == NO_PIECE) empty++;
			else {
				fen << (empty == 0 ? "" : std::to_string(empty))
					<< PIECE_STR[p];
				empty = 0;
			}
		}

		if (empty != 0) fen << empty;
		if (i > 0) fen << '/';
	}

	fen << (side_to_play == WHITE ? " w " : " b ")
		<< (history[game_ply].entry & WHITE_OO_MASK ? "" : "K")
		<< (history[game_ply].entry & WHITE_OOO_MASK ? "" : "Q")
		<< (history[game_ply].entry & BLACK_OO_MASK ? "" : "k")
		<< (history[game_ply].entry & BLACK_OOO_MASK ? "" : "q")
		<< (history[game_ply].entry & ALL_CASTLING_MASK ? "- " : "")
		<< (history[game_ply].epsq == NO_SQUARE ? " -" : SQSTR[history[game_ply].epsq]);

	return fen.str();
}

	

//Moves a piece to a (possibly empty) square on the board and updates the hash
void Position::move_piece(Square from, Square to) {
	hash ^= zobrist::zobrist_table[board[from]][from] ^ zobrist::zobrist_table[board[from]][to]
		^ zobrist::zobrist_table[board[to]][to];
	Bitboard mask = SQUARE_BB[from] | SQUARE_BB[to];
	piece_bb[board[from]] ^= mask;
	piece_bb[board[to]] &= ~mask;
	board[to] = board[from];
	board[from] = NO_PIECE;
}

//Moves a piece to an empty square. Note that it is an error if the <to> square contains a piece
void Position::move_piece_quiet(Square from, Square to) {
	hash ^= zobrist::zobrist_table[board[from]][from] ^ zobrist::zobrist_table[board[from]][to];
	piece_bb[board[from]] ^= (SQUARE_BB[from] | SQUARE_BB[to]);
	board[to] = board[from];
	board[from] = NO_PIECE;
}

#include <iostream>

//Lookup tables of square names in algebraic chess notation
const char* SQSTR[65] = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	"None"
};

//All masks have been generated from a Java program

//Precomputed file masks
const Bitboard MASK_FILE[8] = {
	0x101010101010101, 0x202020202020202, 0x404040404040404, 0x808080808080808,
	0x1010101010101010, 0x2020202020202020, 0x4040404040404040, 0x8080808080808080,
};

//Precomputed rank masks
const Bitboard MASK_RANK[8] = {
	0xff, 0xff00, 0xff0000, 0xff000000,
	0xff00000000, 0xff0000000000, 0xff000000000000, 0xff00000000000000
};

//Precomputed diagonal masks
const Bitboard MASK_DIAGONAL[15] = {
	0x80, 0x8040, 0x804020,
	0x80402010, 0x8040201008, 0x804020100804,
	0x80402010080402, 0x8040201008040201, 0x4020100804020100,
	0x2010080402010000, 0x1008040201000000, 0x804020100000000,
	0x402010000000000, 0x201000000000000, 0x100000000000000,
};

//Precomputed anti-diagonal masks
const Bitboard MASK_ANTI_DIAGONAL[15] = {
	0x1, 0x102, 0x10204,
	0x1020408, 0x102040810, 0x10204081020,
	0x1020408102040, 0x102040810204080, 0x204081020408000,
	0x408102040800000, 0x810204080000000, 0x1020408000000000,
	0x2040800000000000, 0x4080000000000000, 0x8000000000000000,
};

//Precomputed square masks
const Bitboard SQUARE_BB[65] = {
	0x1, 0x2, 0x4, 0x8,
	0x10, 0x20, 0x40, 0x80,
	0x100, 0x200, 0x400, 0x800,
	0x1000, 0x2000, 0x4000, 0x8000,
	0x10000, 0x20000, 0x40000, 0x80000,
	0x100000, 0x200000, 0x400000, 0x800000,
	0x1000000, 0x2000000, 0x4000000, 0x8000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000,
	0x100000000, 0x200000000, 0x400000000, 0x800000000,
	0x1000000000, 0x2000000000, 0x4000000000, 0x8000000000,
	0x10000000000, 0x20000000000, 0x40000000000, 0x80000000000,
	0x100000000000, 0x200000000000, 0x400000000000, 0x800000000000,
	0x1000000000000, 0x2000000000000, 0x4000000000000, 0x8000000000000,
	0x10000000000000, 0x20000000000000, 0x40000000000000, 0x80000000000000,
	0x100000000000000, 0x200000000000000, 0x400000000000000, 0x800000000000000,
	0x1000000000000000, 0x2000000000000000, 0x4000000000000000, 0x8000000000000000,
	0x0
};

//Prints the bitboard, little-endian format
void print_bitboard(Bitboard b) {
	for (int i = 56; i >= 0; i -= 8) {
		for (int j = 0; j < 8; j++)
			std::cout << (char)(((b >> (i + j)) & 1) + '0') << " ";
		std::cout << "\n";
	}
	std::cout << "\n";
}

const Bitboard k1 = 0x5555555555555555;
const Bitboard k2 = 0x3333333333333333;
const Bitboard k4 = 0x0f0f0f0f0f0f0f0f;
const Bitboard kf = 0x0101010101010101;

//Returns number of set bits in the bitboard
//gk inline int pop_count(Bitboard x) {
int pop_count(Bitboard x) {
	x = x - ((x >> 1) & k1);
	x = (x & k2) + ((x >> 2) & k2);
	x = (x + (x >> 4)) & k4;
	x = (x * kf) >> 56;
	return int(x);
}

//Returns number of set bits in the bitboard. Faster than pop_count(x) when the bitboard has few set bits
//gk inline int sparse_pop_count(Bitboard x) {
int sparse_pop_count(Bitboard x) {
	int count = 0;
	while (x) {
		count++;
		x &= x - 1;
	}
	return count;
}

//gk const int DEBRUIJN64[64] = {
constexpr int DEBRUIJN64[64] = {
	0, 47,  1, 56, 48, 27,  2, 60,
   57, 49, 41, 37, 28, 16,  3, 61,
   54, 58, 35, 52, 50, 42, 21, 44,
   38, 32, 29, 23, 17, 11,  4, 62,
   46, 55, 26, 59, 40, 36, 15, 53,
   34, 51, 20, 43, 31, 22, 10, 45,
   25, 39, 14, 33, 19, 30,  9, 24,
   13, 18,  8, 12,  7,  6,  5, 63
};

const Bitboard MAGIC = 0x03f79d71b4cb0a89;

//Returns the index of the least significant bit in the bitboard
//constexpr Square bsf(Bitboard b) {
Square bsf(Bitboard b) {
	return Square(DEBRUIJN64[MAGIC * (b ^ (b - 1)) >> 58]);
}

//Returns the index of the least significant bit in the bitboard, and removes the bit from the bitboard
//gk should be taken to the include file when declared "inline"
//gk inline Square pop_lsb(Bitboard* b) {
Square pop_lsb(Bitboard* b) {
	int lsb = bsf(*b);
	*b &= *b - 1;
	return Square(lsb);
}

//Returns the representation of the move type in algebraic chess notation. (capture) is used for debugging
const char* MOVE_TYPESTR[16] = {
	"", "", " O-O", " O-O-O", "N", "B", "R", "Q", " (capture)", "", " e.p.", "",
	"N", "B", "R", "Q"
};

//Prints the move
//For example: e5d6 (capture); a7a8R; O-O
std::ostream& operator<<(std::ostream& os, const Move& m) {
	os << SQSTR[m.from()] << SQSTR[m.to()] << MOVE_TYPESTR[m.flags()];
	return os;
}

#include <iostream>
#include <cstring>	//gk memcpy() 

//All piece tables are generated from a program written in Java

//A lookup table for king move bitboards
const Bitboard KING_ATTACKS[64] = {
	0x302, 0x705, 0xe0a, 0x1c14,
	0x3828, 0x7050, 0xe0a0, 0xc040,
	0x30203, 0x70507, 0xe0a0e, 0x1c141c,
	0x382838, 0x705070, 0xe0a0e0, 0xc040c0,
	0x3020300, 0x7050700, 0xe0a0e00, 0x1c141c00,
	0x38283800, 0x70507000, 0xe0a0e000, 0xc040c000,
	0x302030000, 0x705070000, 0xe0a0e0000, 0x1c141c0000,
	0x3828380000, 0x7050700000, 0xe0a0e00000, 0xc040c00000,
	0x30203000000, 0x70507000000, 0xe0a0e000000, 0x1c141c000000,
	0x382838000000, 0x705070000000, 0xe0a0e0000000, 0xc040c0000000,
	0x3020300000000, 0x7050700000000, 0xe0a0e00000000, 0x1c141c00000000,
	0x38283800000000, 0x70507000000000, 0xe0a0e000000000, 0xc040c000000000,
	0x302030000000000, 0x705070000000000, 0xe0a0e0000000000, 0x1c141c0000000000,
	0x3828380000000000, 0x7050700000000000, 0xe0a0e00000000000, 0xc040c00000000000,
	0x203000000000000, 0x507000000000000, 0xa0e000000000000, 0x141c000000000000,
	0x2838000000000000, 0x5070000000000000, 0xa0e0000000000000, 0x40c0000000000000,
};

//A lookup table for knight move bitboards
const Bitboard KNIGHT_ATTACKS[64] = {
	0x20400, 0x50800, 0xa1100, 0x142200,
	0x284400, 0x508800, 0xa01000, 0x402000,
	0x2040004, 0x5080008, 0xa110011, 0x14220022,
	0x28440044, 0x50880088, 0xa0100010, 0x40200020,
	0x204000402, 0x508000805, 0xa1100110a, 0x1422002214,
	0x2844004428, 0x5088008850, 0xa0100010a0, 0x4020002040,
	0x20400040200, 0x50800080500, 0xa1100110a00, 0x142200221400,
	0x284400442800, 0x508800885000, 0xa0100010a000, 0x402000204000,
	0x2040004020000, 0x5080008050000, 0xa1100110a0000, 0x14220022140000,
	0x28440044280000, 0x50880088500000, 0xa0100010a00000, 0x40200020400000,
	0x204000402000000, 0x508000805000000, 0xa1100110a000000, 0x1422002214000000,
	0x2844004428000000, 0x5088008850000000, 0xa0100010a0000000, 0x4020002040000000,
	0x400040200000000, 0x800080500000000, 0x1100110a00000000, 0x2200221400000000,
	0x4400442800000000, 0x8800885000000000, 0x100010a000000000, 0x2000204000000000,
	0x4020000000000, 0x8050000000000, 0x110a0000000000, 0x22140000000000,
	0x44280000000000, 0x0088500000000000, 0x0010a00000000000, 0x20400000000000
};

//A lookup table for white pawn move bitboards
const Bitboard WHITE_PAWN_ATTACKS[64] = {
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
	0x200000000000000, 0x500000000000000, 0xa00000000000000, 0x1400000000000000,
	0x2800000000000000, 0x5000000000000000, 0xa000000000000000, 0x4000000000000000,
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
};

//A lookup table for black pawn move bitboards
const Bitboard BLACK_PAWN_ATTACKS[64] = {
	0x0, 0x0, 0x0, 0x0,
	0x0, 0x0, 0x0, 0x0,
	0x2, 0x5, 0xa, 0x14,
	0x28, 0x50, 0xa0, 0x40,
	0x200, 0x500, 0xa00, 0x1400,
	0x2800, 0x5000, 0xa000, 0x4000,
	0x20000, 0x50000, 0xa0000, 0x140000,
	0x280000, 0x500000, 0xa00000, 0x400000,
	0x2000000, 0x5000000, 0xa000000, 0x14000000,
	0x28000000, 0x50000000, 0xa0000000, 0x40000000,
	0x200000000, 0x500000000, 0xa00000000, 0x1400000000,
	0x2800000000, 0x5000000000, 0xa000000000, 0x4000000000,
	0x20000000000, 0x50000000000, 0xa0000000000, 0x140000000000,
	0x280000000000, 0x500000000000, 0xa00000000000, 0x400000000000,
	0x2000000000000, 0x5000000000000, 0xa000000000000, 0x14000000000000,
	0x28000000000000, 0x50000000000000, 0xa0000000000000, 0x40000000000000,
};

//Reverses a bitboard                        
Bitboard reverse(Bitboard b) {
	//gk additional parentheses around arithmetic in operand of ‘|’
	b = (b & 0x5555555555555555) << 1 | ((b >> 1) & 0x5555555555555555);
	b = (b & 0x3333333333333333) << 2 | ((b >> 2) & 0x3333333333333333);
	b = (b & 0x0f0f0f0f0f0f0f0f) << 4 | ((b >> 4) & 0x0f0f0f0f0f0f0f0f);
	b = (b & 0x00ff00ff00ff00ff) << 8 | ((b >> 8) & 0x00ff00ff00ff00ff);

	return (b << 48) | ((b & 0xffff0000) << 16) |
		((b >> 16) & 0xffff0000) | (b >> 48);
}

//Calculates sliding attacks from a given square, on a given axis, taking into
//account the blocking pieces. This uses the Hyperbola Quintessence Algorithm.
Bitboard sliding_attacks(Square square, Bitboard occ, Bitboard mask) {
	return (((mask & occ) - SQUARE_BB[square] * 2) ^
		reverse(reverse(mask & occ) - reverse(SQUARE_BB[square]) * 2)) & mask;
}

//Returns rook attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
Bitboard get_rook_attacks_for_init(Square square, Bitboard occ) {
	return sliding_attacks(square, occ, MASK_FILE[file_of(square)]) |
		sliding_attacks(square, occ, MASK_RANK[rank_of(square)]);
}

Bitboard ROOK_ATTACK_MASKS[64];
int ROOK_ATTACK_SHIFTS[64];
Bitboard ROOK_ATTACKS[64][4096];

const Bitboard ROOK_MAGICS[64] = {
	0x0080001020400080, 0x0040001000200040, 0x0080081000200080, 0x0080040800100080,
	0x0080020400080080, 0x0080010200040080, 0x0080008001000200, 0x0080002040800100,
	0x0000800020400080, 0x0000400020005000, 0x0000801000200080, 0x0000800800100080,
	0x0000800400080080, 0x0000800200040080, 0x0000800100020080, 0x0000800040800100,
	0x0000208000400080, 0x0000404000201000, 0x0000808010002000, 0x0000808008001000,
	0x0000808004000800, 0x0000808002000400, 0x0000010100020004, 0x0000020000408104,
	0x0000208080004000, 0x0000200040005000, 0x0000100080200080, 0x0000080080100080,
	0x0000040080080080, 0x0000020080040080, 0x0000010080800200, 0x0000800080004100,
	0x0000204000800080, 0x0000200040401000, 0x0000100080802000, 0x0000080080801000,
	0x0000040080800800, 0x0000020080800400, 0x0000020001010004, 0x0000800040800100,
	0x0000204000808000, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
	0x0000040008008080, 0x0000020004008080, 0x0000010002008080, 0x0000004081020004,
	0x0000204000800080, 0x0000200040008080, 0x0000100020008080, 0x0000080010008080,
	0x0000040008008080, 0x0000020004008080, 0x0000800100020080, 0x0000800041000080,
	0x00FFFCDDFCED714A, 0x007FFCDDFCED714A, 0x003FFFCDFFD88096, 0x0000040810002101,
	0x0001000204080011, 0x0001000204000801, 0x0001000082000401, 0x0001FFFAABFAD1A2
};

//Initializes the magic lookup table for rooks
void initialise_rook_attacks() {
	Bitboard edges, subset, index;

	for (Square sq = a1; sq <= h8; ++sq) {
		edges = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rank_of(sq)]) |
			((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[file_of(sq)]);
		ROOK_ATTACK_MASKS[sq] = (MASK_RANK[rank_of(sq)]
			^ MASK_FILE[file_of(sq)]) & ~edges;
		ROOK_ATTACK_SHIFTS[sq] = 64 - pop_count(ROOK_ATTACK_MASKS[sq]);

		subset = 0;
		do {
			index = subset;
			index = index * ROOK_MAGICS[sq];
			index = index >> ROOK_ATTACK_SHIFTS[sq];
			ROOK_ATTACKS[sq][index] = get_rook_attacks_for_init(sq, subset);
			subset = (subset - ROOK_ATTACK_MASKS[sq]) & ROOK_ATTACK_MASKS[sq];
		} while (subset);
	}
}

//Returns the attacks bitboard for a rook at a given square, using the magic lookup table
//gk constexpr Bitboard get_rook_attacks(Square square, Bitboard occ) {
Bitboard get_rook_attacks(Square square, Bitboard occ) {
	return ROOK_ATTACKS[square][((occ & ROOK_ATTACK_MASKS[square]) * ROOK_MAGICS[square])
		>> ROOK_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a rook at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
Bitboard get_xray_rook_attacks(Square square, Bitboard occ, Bitboard blockers) {
	Bitboard attacks = get_rook_attacks(square, occ);
	blockers &= attacks;
	return attacks ^ get_rook_attacks(square, occ ^ blockers);
}

//Returns bishop attacks from a given square, using the Hyperbola Quintessence Algorithm. Only used to initialize
//the magic lookup table
Bitboard get_bishop_attacks_for_init(Square square, Bitboard occ) {
	return sliding_attacks(square, occ, MASK_DIAGONAL[diagonal_of(square)]) |
		sliding_attacks(square, occ, MASK_ANTI_DIAGONAL[anti_diagonal_of(square)]);
}

Bitboard BISHOP_ATTACK_MASKS[64];
int BISHOP_ATTACK_SHIFTS[64];
Bitboard BISHOP_ATTACKS[64][512];

const Bitboard BISHOP_MAGICS[64] = {
	0x0002020202020200, 0x0002020202020000, 0x0004010202000000, 0x0004040080000000,
	0x0001104000000000, 0x0000821040000000, 0x0000410410400000, 0x0000104104104000,
	0x0000040404040400, 0x0000020202020200, 0x0000040102020000, 0x0000040400800000,
	0x0000011040000000, 0x0000008210400000, 0x0000004104104000, 0x0000002082082000,
	0x0004000808080800, 0x0002000404040400, 0x0001000202020200, 0x0000800802004000,
	0x0000800400A00000, 0x0000200100884000, 0x0000400082082000, 0x0000200041041000,
	0x0002080010101000, 0x0001040008080800, 0x0000208004010400, 0x0000404004010200,
	0x0000840000802000, 0x0000404002011000, 0x0000808001041000, 0x0000404000820800,
	0x0001041000202000, 0x0000820800101000, 0x0000104400080800, 0x0000020080080080,
	0x0000404040040100, 0x0000808100020100, 0x0001010100020800, 0x0000808080010400,
	0x0000820820004000, 0x0000410410002000, 0x0000082088001000, 0x0000002011000800,
	0x0000080100400400, 0x0001010101000200, 0x0002020202000400, 0x0001010101000200,
	0x0000410410400000, 0x0000208208200000, 0x0000002084100000, 0x0000000020880000,
	0x0000001002020000, 0x0000040408020000, 0x0004040404040000, 0x0002020202020000,
	0x0000104104104000, 0x0000002082082000, 0x0000000020841000, 0x0000000000208800,
	0x0000000010020200, 0x0000000404080200, 0x0000040404040400, 0x0002020202020200
};

//Initializes the magic lookup table for bishops
void initialise_bishop_attacks() {
	Bitboard edges, subset, index;

	for (Square sq = a1; sq <= h8; ++sq) {
		edges = ((MASK_RANK[AFILE] | MASK_RANK[HFILE]) & ~MASK_RANK[rank_of(sq)]) |
			((MASK_FILE[AFILE] | MASK_FILE[HFILE]) & ~MASK_FILE[file_of(sq)]);
		BISHOP_ATTACK_MASKS[sq] = (MASK_DIAGONAL[diagonal_of(sq)]
			^ MASK_ANTI_DIAGONAL[anti_diagonal_of(sq)]) & ~edges;
		BISHOP_ATTACK_SHIFTS[sq] = 64 - pop_count(BISHOP_ATTACK_MASKS[sq]);

		subset = 0;
		do {
			index = subset;
			index = index * BISHOP_MAGICS[sq];
			index = index >> BISHOP_ATTACK_SHIFTS[sq];
			BISHOP_ATTACKS[sq][index] = get_bishop_attacks_for_init(sq, subset);
			subset = (subset - BISHOP_ATTACK_MASKS[sq]) & BISHOP_ATTACK_MASKS[sq];
		} while (subset);
	}
}

//Returns the attacks bitboard for a bishop at a given square, using the magic lookup table
//gk constexpr Bitboard get_bishop_attacks(Square square, Bitboard occ) {
Bitboard get_bishop_attacks(Square square, Bitboard occ) {
	return BISHOP_ATTACKS[square][((occ & BISHOP_ATTACK_MASKS[square]) * BISHOP_MAGICS[square])
		>> BISHOP_ATTACK_SHIFTS[square]];
}

//Returns the 'x-ray attacks' for a bishop at a given square. X-ray attacks cover squares that are not immediately
//accessible by the rook, but become available when the immediate blockers are removed from the board 
Bitboard get_xray_bishop_attacks(Square square, Bitboard occ, Bitboard blockers) {
	Bitboard attacks = get_bishop_attacks(square, occ);
	blockers &= attacks;
	return attacks ^ get_bishop_attacks(square, occ ^ blockers);
}

Bitboard SQUARES_BETWEEN_BB[64][64];

//Initializes the lookup table for the bitboard of squares in between two given squares (0 if the 
//two squares are not aligned)
void initialise_squares_between() {
	Bitboard sqs;
	for (Square sq1 = a1; sq1 <= h8; ++sq1)
		for (Square sq2 = a1; sq2 <= h8; ++sq2) {
			sqs = SQUARE_BB[sq1] | SQUARE_BB[sq2];
			if (file_of(sq1) == file_of(sq2) || rank_of(sq1) == rank_of(sq2))
				SQUARES_BETWEEN_BB[sq1][sq2] =
				get_rook_attacks_for_init(sq1, sqs) & get_rook_attacks_for_init(sq2, sqs);
			else if (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2))
				SQUARES_BETWEEN_BB[sq1][sq2] =
				get_bishop_attacks_for_init(sq1, sqs) & get_bishop_attacks_for_init(sq2, sqs);
		}
}

Bitboard LINE[64][64];

//Initializes the lookup table for the bitboard of all squares along the line of two given squares (0 if the 
//two squares are not aligned)
void initialise_line() {
	for (Square sq1 = a1; sq1 <= h8; ++sq1)
		for (Square sq2 = a1; sq2 <= h8; ++sq2) {
			if (file_of(sq1) == file_of(sq2) || rank_of(sq1) == rank_of(sq2))
				LINE[sq1][sq2] =
				//gk additional parentheses
				(get_rook_attacks_for_init(sq1, 0) & get_rook_attacks_for_init(sq2, 0))
				| SQUARE_BB[sq1] | SQUARE_BB[sq2];
			else if (diagonal_of(sq1) == diagonal_of(sq2) || anti_diagonal_of(sq1) == anti_diagonal_of(sq2))
				LINE[sq1][sq2] =
				//gk additional parentheses
				(get_bishop_attacks_for_init(sq1, 0) & get_bishop_attacks_for_init(sq2, 0))
				| SQUARE_BB[sq1] | SQUARE_BB[sq2];
		}
}

Bitboard PAWN_ATTACKS[NCOLORS][NSQUARES];
Bitboard PSEUDO_LEGAL_ATTACKS[NPIECE_TYPES][NSQUARES];

//Initializes the table containg pseudolegal attacks of each piece for each square. This does not include blockers
//for sliding pieces
void initialise_pseudo_legal() {
	memcpy(PAWN_ATTACKS[WHITE], WHITE_PAWN_ATTACKS, sizeof(WHITE_PAWN_ATTACKS));
	memcpy(PAWN_ATTACKS[BLACK], BLACK_PAWN_ATTACKS, sizeof(BLACK_PAWN_ATTACKS));
	memcpy(PSEUDO_LEGAL_ATTACKS[KNIGHT], KNIGHT_ATTACKS, sizeof(KNIGHT_ATTACKS));
	memcpy(PSEUDO_LEGAL_ATTACKS[KING], KING_ATTACKS, sizeof(KING_ATTACKS));
	for (Square s = a1; s <= h8; ++s) {
		PSEUDO_LEGAL_ATTACKS[ROOK][s] = get_rook_attacks_for_init(s, 0);
		PSEUDO_LEGAL_ATTACKS[BISHOP][s] = get_bishop_attacks_for_init(s, 0);
		PSEUDO_LEGAL_ATTACKS[QUEEN][s] = PSEUDO_LEGAL_ATTACKS[ROOK][s] |
			PSEUDO_LEGAL_ATTACKS[BISHOP][s];
	}
}

//Initializes lookup tables for rook moves, bishop moves, in-between squares, aligned squares and pseudolegal moves
void initialise_all_databases() {
	initialise_rook_attacks();
	initialise_bishop_attacks();
	initialise_squares_between();
	initialise_line();
	initialise_pseudo_legal();
}
