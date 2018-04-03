
// =====================================================================
// FILE:        MyAI.cpp
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

#include "MyAI.hpp"


using namespace std;


MyAI::MyAI() : Agent()
{
	xcoor = 0;
	ycoor = 0;
	dir = 1;
	xbound = 6;
	ybound = 6;

	gohome = 0;
	num_moves = 0;
	
	shot_arrow = 0;
	wumpus_dead = 0;

	move = FORWARD;
}	
// Main move function
Agent::Action MyAI::getAction
(
	bool stench,
	bool breeze,
	bool glitter,
	bool bump,
	bool scream
)
{
	if( gohome==1 ) {
		if(xcoor==0 && ycoor==0) {
			return CLIMB;
		}
	}
	
	
	// 1. Check if step_queue empty
	if( step_queue.empty() ) {	

		// 3. Update bound, account for visits of pair(x,y) if bump
		if(bump) {
			proc_bump();
			--known[make_pair(xcoor, ycoor)].second;
			proc_move(TURN_LEFT);
			return TURN_LEFT;
		}

		// 4. Make pair of current coor
		pair<int, int> xypair = std::make_pair(xcoor, ycoor); 		

		// 5. If glitter actuator, grab, go home.
		if( glitter ) {
			gohome = 1;
			clear_step_queue();
			direct_path(path_home());
			return GRAB;
		}

		// 6. Find unknown neighbors (coors not in known map) and insert into potential map w/val
		vector< pair< int,int> > unknown = neighbors_unknown(xcoor, ycoor);
		for( vector< pair<int,int>>::iterator it=unknown.begin(); it!=unknown.end(); ++it) {
			potential[*it] = (breeze * 4);
			if(wumpus_dead<1) potential[*it] += (stench * 8);
		}

		// 7. Insert curr xy coor into known map
		if( known.count(xypair)>0 ) {
			++known[xypair].second;
		} else {
			known[xypair].first += breeze;
			if(!wumpus_dead) known[xypair].first += (2 * stench);
		}

		// 7.5 If stench, shoot arrow. If scream, update potential map.
		if( stench && shot_arrow<1 ) {
			shot_arrow = 1;
			return SHOOT;
		}
		if( scream ) {
			remove_stenches();
			wumpus_dead = 1;
		}

		check_impossible();	

		// 8. Find next coor to go to.
		pair<int,int> next_coor = get_next(neighbors(xcoor, ycoor));
//		printf("Next Coord: (%d, %d)\n", next_coor.first, next_coor.second);
		
		// 9. Only possible when there are no moves possible. (B/S at (0,0))
		if(next_coor.first > 10) {
			return CLIMB;
		}
		
	
		// 10. Fills step_queue with moves to next_step coor.
		get_moves(xypair, next_coor, dir);
	}

	// 11. Updates PMap with truth from KMap	
	upd_potential();

	//  =========== Printing Maps ===========
	//  When debugging, the print stmts for a state will be after said state
//		printf("\nCurrent Coors & Dir: (%d, %d) : %d\n", xcoor, ycoor, dir);
//		printf("GoHome: %d\n", gohome);
//		print_known();
//		print_potential();
		
		if(gohome==1) {	
//			print_vector(path_home());
//			print_queue(step_queue);	
		}
	//  =====================================
	// need to process updating coords
	

	move = step_queue.front();
	step_queue.pop();
	//printf("Move from queue: %d\n\n", move);

	
	proc_move(move);
	return move;
		
}

// Heuristic Function
/** updates all heuristic evaluations for known coords
int MyAI::update_heur( std::pair<int, int> coords )
{
	////////
	/////////
	/////////
	return 0;
}
*/


void MyAI::check_impossible()
{
	int imp = 1;
	for(map<pair<int,int>, int>::iterator it = potential.begin(); it!=potential.end(); ++it) {
		//printf("\tFunction check_impossible: %d\n", it->second);
		if(it->second <= 3) {
			imp = 0;
			break;
		}		
	}
	if (imp>0){
		gohome = 1;
		//printf("\nGOHOME = 1 from check_imp\n");
	}
}

// upd_potential() -> void : updates potential map
void MyAI::upd_potential()
{
	// remove coor that aren't in bounds
	
	
	// removes coor from potential if in known( visited )
	map<pair<int,int>, pair<int,int>>::iterator it;
	for( it = known.begin(); it != known.end(); ++it ) {
		if( potential.count(it->first)==1 ) {
			potential.erase(it->first);
		}
	}
	
	map<pair<int,int>, int>::iterator pot_it;
	for( pot_it = potential.begin(); pot_it!=potential.end(); ++pot_it )
	{
		if(pot_it->first.first >= xbound || pot_it->first.second >= ybound ) {
			potential.erase(pot_it->first);
		}
		
		if(pot_it->second > 3) {
			vector<pair<int, int>> nk = neighbors_known((pot_it->first).first, (pot_it->first).second);
			if(val_contains(pot_it->second, 4)) {
				for( vector< pair<int, int> >::iterator n = nk.begin(); n!=nk.end(); ++n) {
					if(!val_contains(known[*n].first, 1)) {
						potential[pot_it->first] -= 4;
					}
				}
			}
			if(val_contains(pot_it->second, 8)) {
				for( vector< pair<int, int> >::iterator n = nk.begin(); n!=nk.end(); ++n) {
					if(!val_contains(known[*n].first, 2)) {
						potential[pot_it->first] -= 8;
					}
				}
			}
		}	
	}
/*
	pair<int,int> coord;
	int pit_count = 0;
	int wump_count = 0;
	for( it=known.begin(); it!=known.end(); ++it ) {
		if( (*it).second.first>0 ) {
			int known_x = (*it).first.first;
			int known_y = (*it).first.second;
			
			vector<pair<int, int>> known_neigh = neighbors_known(known_x, known_y);
			
			if(known_neigh.size()==3 || (is_touch_corner(known_x, known_y) && known_neigh.size()==1) || is_touch_side(known_x,known_y) && known_neigh.size()==2) {
				printf("Deducting... (%d, %d)\n", known_x, known_y);
				
				coord = neighbors_unknown(known_x, known_y).front();
				printf("1\n");
				if(val_contains((*it).second.first, 1)) {
					printf("Deduced Pit at (%d, %d) \n\n", coord.first, coord.second);
					++pit_count;
				}
				if(val_contains((*it).second.first, 2)) {
					printf("Deduced Wumpus at (%d, %d) \n\n", coord.first, coord.second);
					++wump_count;
				}
				
				printf("2\n");
				
				if(pit_count==1 || wump_count==1) break;
			}
		}
	}
	
	if(pit_count==1 || wump_count==1) {
	
		if(pit_count==1) {
			known[coord].first += 4;
		}
		if(wump_count==1) {
			known[coord].first += 8;
		}
		printf("Recursive call upd_potential();\n");
		upd_potential();
	}
*/
}

void MyAI::clear_step_queue() {
	queue<Agent::Action> empty;
	step_queue.swap(empty);
}

// val_contains(int, int) -> bool : return whether type is in val
bool MyAI::val_contains( int val, int type ) {
	int temp_type = 0;
	for( int e=3; e>=0; --e) {
		temp_type = pow(2,e);
		if( val >= temp_type ){
			if( type == temp_type ) return 1;
			val -= temp_type;
		}
		if( val==0 ) break;
	}
	return 0;
}

// get_moves( pair<int,int> ) -> void
int MyAI::get_moves( pair<int,int> curr_xy, pair<int, int> fin_xy, int curr_dir ) {
	int xdiff = fin_xy.first - curr_xy.first;
	int ydiff = fin_xy.second - curr_xy.second;
	int next_dir = 0;
	
	if(xdiff<0) next_dir = 3;
	if(xdiff>0) next_dir = 1;
	if(ydiff<0) next_dir = 2;
	if(ydiff>0) next_dir = 0;

	int temp_move = curr_dir - next_dir;
	if( temp_move==1 || temp_move==-3 ) step_queue.push(TURN_LEFT);
	if( temp_move==-1 || temp_move==3 ) step_queue.push(TURN_RIGHT);
	if( temp_move==2 || temp_move ==-2 ) {
		step_queue.push(TURN_LEFT);
		step_queue.push(TURN_LEFT);
	}
	step_queue.push(FORWARD);
	
	return next_dir;
}


void MyAI::direct_path( vector<pair<int,int>> path_vector ) {
	pair<int,int> tmp_pair = make_pair(xcoor, ycoor);
	int tmp_dir = dir;
	vector<pair<int,int>>::reverse_iterator vect_it;
	for(vect_it=path_vector.rbegin(); vect_it!=path_vector.rend(); vect_it++) {
//		printf(" move to (%d, %d)\n", (*vect_it).first, (*vect_it).second);
		tmp_dir = get_moves(tmp_pair, *vect_it, tmp_dir);	
		tmp_pair = *vect_it;
	}
}

vector<pair<int,int>> MyAI::path_home() {
	queue<pair<int,int>> frontier;
	map<pair<int,int>, pair<int,int>> path_map;

	pair<int,int> current_xy = make_pair(xcoor, ycoor);
	pair<int,int> tmp_pair;
	vector<pair<int,int>> known_pairs;
	vector<pair<int,int>>::iterator known_it;

	frontier.push(current_xy);

	bool success = false;

	// until frontier is empty, expand each frontier node and map with path_map to parent coord;
	while(!frontier.empty()) {
		tmp_pair = frontier.front();
		frontier.pop();	
		known_pairs = neighbors_known(tmp_pair.first, tmp_pair.second);
		for(known_it = known_pairs.begin(); known_it!=known_pairs.end(); ++known_it) {
			if(path_map.count(*known_it)==0){
				frontier.push(*known_it);
				path_map[*known_it] = tmp_pair;
			}
			if((*known_it).first==0 && (*known_it).second==0) success = true;
		}
	}
	
	vector<pair<int,int>> path_vector;

	if(!success) return path_vector;
	
	tmp_pair = make_pair(0,0);
	while(tmp_pair!=current_xy) {	
		path_vector.push_back(tmp_pair);
		tmp_pair = path_map[tmp_pair];
	}	

	return path_vector;
	
}


//===========================================================================
//===================== Get Neighbors and Next Best Coor ====================
//===========================================================================

// get_next() -> std::pair<int,int> : return best next step
pair<int, int> MyAI::get_next( vector<pair<int,int>> n  ) {
	
	if(gohome) {
		if( (xcoor==0 && ycoor==1) || (xcoor==1 && ycoor==0)) return make_pair(0,0);
	}

	bool next_found = 0;
	pair<int,int> anspair;

	vector<pair<int,int>>::iterator it;
	for(it = n.begin(); it!=n.end(); ++it) {
		if( potential.count(*it)==1 && potential[(*it)]==0 ) {
			 anspair = *it;
			next_found = 1;
			break; 
		}
	}
	
	if(!next_found) {
		auto compare_func = [](pair<pair<int,int>, int> a, pair<pair<int,int>, int> b)
		{
			 return a.second > b.second;
		};

		priority_queue< pair< pair<int,int>, int >, vector<pair<pair<int,int>, int>>, decltype(compare_func)> pq( compare_func );
		
		for(it=n.begin(); it!=n.end(); ++it) {
			if( known.count(*it)==1 && known[*it].first<=3 ) {
				pq.push(make_pair(*it, known[*it].second));
				next_found = 1;;
				//################333
			}
		}
		if( next_found>0 ) {
			return pq.top().first;
		} else {
			return make_pair(100,100);
		}

	}

	
	return anspair;
}

// distance( pair<int,int>, pair<int,int> ) -> int : return Manhattan distance
//

// neighbors() gets surrounding coords
vector<pair<int,int>> MyAI::neighbors( int x, int y )
{
	vector<pair<int,int>> ansvector;
	if(x>0)		ansvector.push_back(std::make_pair(x-1, y));
	if(y>0)		ansvector.push_back(std::make_pair(x, y-1));
	if(x<xbound) ansvector.push_back(std::make_pair(x+1, y));
	if(y<ybound) ansvector.push_back(std::make_pair(x, y+1));
	
	return ansvector;
}

// like neighbors but without known coor
std::vector<std::pair<int,int>> MyAI::neighbors_unknown( int x, int y)
{
	std::vector< std::pair< int, int > > ansvector;
	std::pair< int, int > apair;
	if(x>0 && known.count(apair=std::make_pair(x-1, y))==0 ) ansvector.push_back(apair);
	if(y>0 && known.count(apair=std::make_pair(x, y-1))==0 ) ansvector.push_back(apair);
	if(x<xbound && known.count(apair=std::make_pair(x+1, y))==0 ) ansvector.push_back(apair);
	if(y<ybound && known.count(apair=std::make_pair(x, y+1))==0 ) ansvector.push_back(apair);

	return ansvector;
}

// neighbors wihtout unknown coors
vector<pair<int,int>> MyAI::neighbors_known( int x, int y )
{
	vector< std::pair< int, int > > ansvector;
	pair< int, int > apair;	
	if(x>0 && known.count(apair=std::make_pair(x-1, y))==1 ) ansvector.push_back(apair);
	if(y>0 && known.count(apair=std::make_pair(x, y-1))==1 ) ansvector.push_back(apair);
	if(x<xbound && known.count(apair=std::make_pair(x+1, y))==1 ) ansvector.push_back(apair);
	if(y<ybound && known.count(apair=std::make_pair(x, y+1))==1 ) ansvector.push_back(apair);

	return ansvector;
}

//=======================================================
//================ Coor & Dir Functions =================
//=======================================================

// Remove all stenches indicators in potential map
void MyAI::remove_stenches() {
	map<pair<int,int>, int>::iterator st_it;
	for(st_it = potential.begin(); st_it!=potential.end(); ++st_it) {
		if(val_contains(st_it->second, 8)) {
			potential[st_it->first] -= 8;
			//printf(" Removed stench from (%d, %d)\n", st_it->first.first, st_it->first.second);
		}
	}
}
// Updates coors and dir given Action from current state
void MyAI::proc_move( Agent::Action a ) {
	switch(a) {
		case TURN_LEFT:
			proc_tl();
			break;
		case TURN_RIGHT:
			proc_tr();
			break;
		case FORWARD:
			proc_forward();
			break;		
	}
}

// Process direction var for left-turn
void MyAI::proc_tl() {
	if(dir==0) dir = 4;
	dir--;
	
}

// Process dir var for right-turn
void MyAI::proc_tr() {
	if(dir==3) dir = -1;
	dir++;
}

// Return coords to previous pre-bump values
void MyAI::proc_bump() {
	if(dir==0){
		ycoor--;
		ybound = ycoor;
	}
	if(dir==1){
		xcoor--;
		xbound = xcoor;
	}
	if(dir>1) printf("Error\n");
	
}
// Process x & y coords for moving given direction
void MyAI::proc_forward() {
	switch(dir) {
		case 0:
			ycoor++;
			break;
		case 1:	
			xcoor++;
			break;
		case 2:
			ycoor--;
			break;
		case 3:
			xcoor--;
			break;
	}		
}

bool MyAI::is_touch_corner(int x, int y) {
	int xy = x+y;
	return xy==0 || xy==xbound*2 || xy==xbound || xy==ybound;
}

bool MyAI::is_touch_side(int x, int y) {
	if(is_touch_corner(x, y)) return false;
	if(x==0 || y==0 || x==xbound || y==ybound) return true;
	return false;
	
}
//=============================================================
//	Printing Functions
//=============================================================
void MyAI::print_vector( vector< pair<int,int> > v ) {
	vector< pair<int, int> >::iterator it;
	printf("Printing Vector\n");
	for( it=v.begin(); it!=v.end(); ++it) {
		printf("\tpair: (%d, %d)\n", (*it).first, (*it).second);
	}
}
	
void MyAI::print_queue( queue<Agent::Action> q ) {
	printf("Printing Step_Queue:\n");
	while(!q.empty()) {
		printf("\t%d\n", q.front());
		q.pop();
	}
}

void MyAI::print_known() {	
	printf("Testing known map: \n");
	for(map<pair<int,int>, pair<int,int>>::iterator it = known.begin(); it!=known.end(); ++it) {
		printf("\t(%d, %d) -> (%d, %d)\n", (it->first).first, (it->first).second, (it->second).first, (it->second).second);
	}	
}

void MyAI::print_potential() {
	printf("Printing potential map: \n");
	for(std::map<std::pair<int,int>, int>::iterator it = potential.begin(); it!=potential.end(); ++it) {
		printf("\t(%d, %d) -> %d\n", (it->first).first, (it->first).second, it->second);
	}	
}
//=============================================================

//**//

