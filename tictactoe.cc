#include <cstdlib>
#include <cassert>

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>


const bool debug_mode= false;

const bool debug_move= false;

#define DEBUG if( debug_mode )
#define DEBUG_MOVE if( debug_move )

struct Move;
std::ostream & operator << ( std::ostream &os, const Move &m );

struct Move
{
	int x;
	int y;

	explicit inline Move() {}

	explicit inline Move( const int space )
		: x( space % 3 ), y( space / 3 )
	{
		DEBUG_MOVE std::cerr << "Move space: " << *this << std::endl;
	}

	explicit inline Move( const int i_x, const int i_y )
		: x( i_x ), y( i_y ) {}
};

std::ostream &
operator << ( std::ostream &os, const Move &m )
{
	os << "{ " << m.x << ", " << m.y << " }";

	return os;
}


class Board
{
	private:
		char board[3][3];

	public:
		explicit inline
		Board() : board() {}

		void
		playMove( const Move &move, const char player )
		{
			board[ move.x ][ move.y ]= player;
		}

		inline const char &
		checkSpace( const Move &move ) const
		{
			return this->board[ move.x ][ move.y ];
		}

		inline const char &
		checkSpace( const int x, const int y ) const
		{
			return this->checkSpace( Move( x, y ) );
		}

		#if 0
		const char [3][3] &
		getBoard() const
		{
			return board;
		}
		#endif

		bool
		isVictory() const
		{
			for( int i= 0; i < 3; ++i )
			{
				if( board[ i ][ 0 ] != 0 &&
				    board[ i ][ 0 ] == board[ i ][ 1 ] && board[ i ][ 1 ] == board[ i ][ 2 ] ) return true;
				if( board[ 0 ][ i ] != 0 && 
				    board[ 0 ][ i ] == board[ 1 ][ i ] && board[ 1 ][ i ] == board[ 2 ][ i ] ) return true;
			}

			if( board[ 0 ][ 0 ] != 0 && 
			    board[ 0 ][ 0 ] == board[ 1 ][ 1 ] && board[ 1 ][ 1 ] == board[ 2 ][ 2 ] ) return true;
			if( board[ 0 ][ 2 ] != 0 && 
			    board[ 0 ][ 2 ] == board[ 1 ][ 1 ] && board[ 1 ][ 1 ] == board[ 2 ][ 0 ] ) return true;

			return false;
		}

		bool
		isValidMove( const Move &move ) const
		{
			DEBUG_MOVE std::cerr << "Checking validity of: " << move << std::endl;

			const bool inRange= move.x >= 0 && move.x < 3
			                 && move.y >= 0 && move.y < 3;
			DEBUG_MOVE
			{
				if( inRange )
				{
					std::cerr << "At that space, we find: " << (int) board[ move.x ][ move.y ] << std::endl;
				}
			}

			const bool rv= inRange && board[ move.x ][ move.y ] == 0;

			DEBUG_MOVE std::cerr << "We've decided that the move is " << ( rv ? "good" : "bad" ) << std::endl;
			return rv;
		}
};

inline std::ostream &
operator << ( std::ostream &os, const Board &b )
{
	char spaceID= '0';
	for( unsigned i= 0; i < 3; ++i )
	{
		for( unsigned j= 0; j < 3; ++j )
		{
			DEBUG
			{
				std::cerr << "Board space " << j << ", " << i << ": "
				          << (int) b.checkSpace( j, i ) << std::endl;
			}

			++spaceID;
			os << ( b.checkSpace( j, i ) ? b.checkSpace( j, i ) : spaceID );
			if( j < 2 ) os << '|';
		}
		os << "\n";
		if( i < 2 ) os << "-----\n";
	}
	return os;
}



class Player
{
	private:
		// Disable copy:
		Player( const Player & );
		Player &operator= ( const Player & );

	protected:
		const char token;

		explicit inline Player( const char i_token ) : token( i_token ) {}

		struct retry_move {};

	private:
		virtual Move makeMove_impl( const Board &b )= 0;
		virtual Move makeMove_impl( const Board &b, const retry_move &retry )= 0;


	public:
		Move
		makeMove( const Board &b )
		{
			Move rv;

			rv= this->makeMove_impl( b );

			DEBUG_MOVE std::cerr << "Made this move: " << rv << std::endl;

			while( !b.isValidMove( rv ) )
			{
				rv= this->makeMove_impl( b, retry_move() );
			}

			assert( b.isValidMove( rv ) );

			return rv;
		}
};

class HumanPlayer
		: public Player
{
	public:
		explicit HumanPlayer( const char i_token ) : Player( i_token ) {}

		virtual Move
		makeMove_impl( const Board &b )
		{
			std::cout << "The current game state is: " << std::endl << b << std::endl;

			std::cout << "Your player token is: " << token << std::endl;

			int space;
			std::cout << "Input a board space, 1 - 9 to move: " << std::flush;
			std::cin >> space;

			space= std::max( space, -space );

			--space;

			return Move( space );
		}

		virtual Move
		makeMove_impl( const Board &b, const retry_move & )
		{
			std::cout << "You've made an invalid move.  Please try again." << std::endl;
			return this->HumanPlayer::makeMove_impl( b );
		}
};

// ASS = Artificial Simple Stupidity, by contrast with AI = Artificial Intelligence
class AssPlayer : public Player {};


inline char
nextPlayer( const int currPlayer )
{
	return 1 - currPlayer;
}

void
playGame()
{
	const char playerTokens[]= { 'X', 'O' };


	// Create the board
	Board b;

	// Create the players
	const std::auto_ptr< Player > p1( new HumanPlayer( 'X' ) );
	const std::auto_ptr< Player > p2( new HumanPlayer( 'O' ) );

	// Prepare to play
	int currPlayer= nextPlayer( 0 );

	std::vector< Player * > players;
	players.push_back( &*p1 );
	players.push_back( &*p2 );

	int moves= 0;

	do
	{
		currPlayer= nextPlayer( currPlayer );
		const Move move= players[ currPlayer ]->makeMove( b );

		assert( b.isValidMove( move ) );
		b.playMove( move, playerTokens[ currPlayer ] );

		if( ++moves == 9 )
		{
			std::cout << "The game is tied." << std::endl;
			return;
		}
	}
	while( !b.isVictory() );

	std::cout << "This game's winner is: " << playerTokens[ currPlayer ] << std::endl;

	std::cout << "Ending board: " << std::endl;
	std::cout << b << std::endl;
}

int 
main()
try
{
	std::string choice;
	do
	{
		playGame();

		std::cout << "Do you want to play again?" << std::endl;
		std::cin >> choice;
	}
	while( choice == "yes" )

	return EXIT_SUCCESS;
}
catch( const std::exception &ex )
{
	std::cerr << "Error: " << ex.what() << std::endl;
	return EXIT_FAILURE;
}
