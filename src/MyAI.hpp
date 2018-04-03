// ======================================================================
// FILE:        MyAI.hpp
//
// AUTHOR:      Abdullah Younis
//
// DESCRIPTION: This file contains your agent class, which you will
//              implement. You are responsible for implementing the
//              'getAction' function and any helper methods you feel you
//              need.
//
// NOTES:       - If you are having trouble understanding how the shell
//                works, look at the other parts of the code, as well as
//                the documentation.
//
//              - You are only allowed to make changes to this portion of
//                the code. Any changes to other portions of the code will
//                be lost when the tournament runs your code.
// ======================================================================

#ifndef MYAI_LOCK
#define MYAI_LOCK


#include "Agent.hpp"
#include <stdlib.h>
#include <map>
#include <utility>
#include <vector>
#include <queue>
#include <math.h>

using namespace std;

class MyAI : public Agent
{
public:
	MyAI ( void );
	
	Action getAction
	(
		bool stench,
		bool breeze,
		bool glitter,
		bool bump,
		bool scream
	);
	
	int xcoor;
	int ycoor;		
	int dir;		//direction it is facing (0-N, 1-E, 2-S, 3-W)
	int turn_times; // num times it should turn
	
	int xbound;		// map limit
	int ybound;
	int num_moves;
	
	int shot_arrow;
	int wumpus_dead;

	bool gohome;	// gohome boolean
	
	Agent::Action  move;
	
	pair<int, int> dest;	//destination coord

    
	map<pair<int, int>, pair<int,int>> known;		// known coords 
	map<pair<int, int>, int> potential;  // potential coords of whole world

	queue<Agent::Action> step_queue;	// future step commands. If not empty, keep popping.

	//	__FUNCTIONS__  //
	//===========================
	void print_known(); 
	void print_potential();
	void print_vector( vector< pair<int, int> > v );
	void print_queue( queue<Agent::Action> q );
	
	void remove_stenches(); 	// after killing wumpus, remove all stenches from pot_map
	void proc_move( Agent::Action a ); // updates coors and dirs given move
	void proc_tl();	// processes vars for turn left
	void proc_tr();	// process vars for turn right
	void proc_forward();	// process vars for forward 
	void proc_bump();	// process bump aka turn around (turn_times = 2)?

	int get_heur( bool stench, bool breeze );// heuristic func for stat analysis.
	pair<int,int> get_next( vector< pair<int,int>> n ); //get next move 

	void direct_path( vector<pair<int,int>> path_vector );
	vector<pair<int,int>> path_home(); // expand all nodes until (0,0). uses map (0,0) -> (0,1)
	void clear_step_queue(); // Clear the path queue
	int get_moves( pair<int,int> curr_xy, pair<int,int> fin_xy, int curr_dir );	//fill_step_queue with moves

	void check_impossible();  // set go_home = 1.
	
	vector<pair<int,int>> neighbors(int x, int y); 	// get surrounding neighbors
	vector<pair<int,int>> neighbors_unknown(int x, int y); //get unknown neighbor
	vector<pair<int,int>> neighbors_known(int x, int y);
	void upd_potential(); // update potential map

	bool is_touch_corner(int x, int y);
	bool is_touch_side(int x, int y);

	bool val_contains(int val, int type); // determine if value contains hazard type


	//===========================
};


#endif
