#include "Player.hh"
using namespace std;

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME UmbreOff_19

struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }


  /**
   * Attributes for your player can be defined here.
   */
  vector<vector<CType> > cells;
  typedef pair<bool, bool> PGP; // Pair(GhostWall?, Present?)
  vector<vector<PGP> > ghost_walls;
  typedef pair<bool, Dir> PVD; // Pair(Visited?, Direction)
  vector<vector<PVD> > fixed_walls; // Used later as visited_cells
  vector<vector<int> > enemies;

  typedef pair<int, Pos> PPP; // Pair(Priority, Position)
  typedef pair<Dir, int> PDD; // Pair(Direction, Distance)
  typedef pair<PPP, PDD> obj; // Pair( Pair(Priority, Position), Pair(Direction, Distance) )
  struct CompareObjectives {
    bool operator()(obj const & obj1, obj const & obj2) {
      return obj1.first.first > obj2.first.first; // (Priority)
    }
  };

  vector<Dir> all_dirs;
  /*vector<Dir> all_dirs2;
  vector<Dir> all_dirs3;
  vector<Dir> all_dirs4;*/
  vector<int> obj_weight;

  int corner = 0;
  int strategy = 0;
  int mode = 0;
  Pos objective;

  bool couts = false;
  //bool couts = true;



  /**
   * Play method.
   *
   * This method will be invoked once per each round.
   * You have to read the board here to place your actions
   * for this round.
   *
   */
  virtual void play () {
    //- Intialization
      const Poquemon& p = poquemon(me());

      if (round() == 0) {
        cells = vector< vector<CType> >(rows(), vector<CType>(cols()));
        ghost_walls = vector< vector<PGP> >(rows(), vector<PGP>(cols(), make_pair(false, false)));
        fixed_walls = vector< vector<PVD> >(rows(), vector<PVD>(cols(), make_pair(false, None)));
        all_dirs = {None, Left, Right, Top, Bottom};
        obj_weight = {0,   // [0] Stone // good on default
                      1,   // [1] 500
                      2,   // [2] 400
                      3,   // [3] 300
                      5,   // [4] 200
                      4,   // [5] Attack or Defense
                      6,   // [6] 100
                      7};  // [7] Scope
        /*obj_weight = {0,   // [0] Stone // good on default
                      0,   // [1] 500
                      0,   // [2] 400
                      0,   // [3] 300
                      0,   // [4] 200
                      0,   // [5] Attack or Defense
                      0,   // [6] 100
                      0};  // [7] Scope*/
      }
      enemies = vector< vector<int> >(rows(), vector<int>(cols(), -1));

      // Save enemy poquemons
      vector<Poquemon> p_enemies;
      for (int i = 0; i < 4; ++i) {
        if (i != me()) {
          p_enemies.push_back(poquemon(i));
        }
      }

      // Mark enemies on the map
      for (int i = 0; (unsigned)i < p_enemies.size(); ++i) {
        enemies[p_enemies[i].pos.i][p_enemies[i].pos.j] = i;
      }

      //* PRINTING
        //if (couts) {
        //  for (int i = 0; (unsigned)i < p_enemies.size(); ++i) {
        //    cout << "poq "<<i<<" "<<p_enemies[i].pos.i<<" "<<p_enemies[i].pos.j<<endl;
        //  }
        //}

    //- Save cells content
      //* PRINTING
      if (couts) cout << "##>> Cells" << endl;
      for (int i = 0; i < rows(); ++i) {
        for (int j = 0; j < cols(); ++j) {
          cells[i][j] = cell_type(i, j);

          // Save ghost walls position
          if (ghost_wall({i, j}) != -1) {
            ghost_walls[i][j].first = true;
            if (cells[i][j] == Empty) ghost_walls[i][j].second = false;
            else ghost_walls[i][j].second = true;

            // I mark ghost walls as empty always because before moving to a cell i check if it's going to be ghost walled
            cells[i][j] = Empty;
          }

          // Save enemies position
          //if (cell_id({i, j}) != -1 and cell_id({i, j}) != me()) enemies[i][j] = true;
          
          // Intialize vector with fixed walls
          if (round() == 0 and cells[i][j] == Wall and (not ghost_walls[i][j].first)) fixed_walls[i][j].first = true;
          
          //* PRINTING
          if (couts) {
            if (p.alive and i == p.pos.i and j == p.pos.j) cout << '*';
            else if (enemies[i][j] != -1) cout << enemies[i][j];
            // Ghost wall not present
            else if (cells[i][j] == Empty) {
              if (ghost_walls[i][j].first) cout << 'g';
              else cout << '.';
            }
            // Ghost wall present
            else if (cells[i][j] == Wall) {
              if (ghost_walls[i][j].first) cout << 'G';
              else {
                cout << 'X';
              }
            }
            else if (cells[i][j] == Point) cout << 'P';
            else if (cells[i][j] == Stone) cout << 'S';
            else if (cells[i][j] == Scope) cout << 'R';
            else if (cells[i][j] == Attack) cout << 'A';
            else if (cells[i][j] == Defense) cout << 'D';
          }
        }
        //* PRINTING
        if (couts) cout << endl;
      }

    //- When maze is known, follow strategy
      if (round() == 0) {
        // When map is known, select strategy
        int n_maze = which_maze();
        if (n_maze == 0) {
          if (pos_eq(p.pos,{2, 6})) corner = 1;
          else if (pos_eq(p.pos,{2, 24})) corner = 2;
          else if (pos_eq(p.pos,{12, 6})) corner = 3;
          else if (pos_eq(p.pos,{12, 24})) corner = 4;

          if (corner != 0) {
            mode = 1;
            strategy = 1;
            objective = {7, 15};
          }
        }
        else if (n_maze == 1) {
          if (pos_eq(p.pos,{1, 1})) corner = 1;
          else if (pos_eq(p.pos,{1, 28})) corner = 2;
          else if (pos_eq(p.pos,{28, 1})) corner = 3;
          else if (pos_eq(p.pos,{28, 28})) corner = 4;

          if (corner != 0) {
            strategy = 2;
            mode = 1;
            
            if (corner == 1) objective = {8, 1};
            else if (corner == 2) objective = {8, 28};
            else if (corner == 3) objective = {21, 1};
            else if (corner == 4) objective = {21, 28};
          }
        }
        else if (n_maze == 2) {
          if (pos_eq(p.pos,{1, 14})) corner = 1;
          else if (pos_eq(p.pos,{1, 16})) corner = 2;
          else if (pos_eq(p.pos,{19, 14})) corner = 3;
          else if (pos_eq(p.pos,{19, 16})) corner = 4;

          if (corner != 0) {
            strategy = 3;
            mode = 1;
          }

          if (corner == 1) objective = {5, 14};
          else if (corner == 2) objective = {5, 16};
          else if (corner == 3) objective = {15, 14};
          else if (corner == 4) objective = {15, 16};
        }
      }

      if (strategy == 1) {
        if (round() == 14) mode = 0;
      }
      else if (strategy == 2) {
        if (round() == 7) {
          if (corner == 1) objective = {8, 2};
          else if (corner == 2) objective = {8, 27};
          else if (corner == 3) objective = {21, 2};
          else if (corner == 4) objective = {21, 27};
        }
        else if (round() == 8) {
          if (corner == 1) objective = {9, 2};
          else if (corner == 2) objective = {9, 27};
          else if (corner == 3) objective = {20, 2};
          else if (corner == 4) objective = {20, 27};
        }
        else if (round() == 9) {
          if (corner == 1) objective = {9, 13};
          else if (corner == 2) objective = {9, 16};
          else if (corner == 3) objective = {20, 13};
          else if (corner == 4) objective = {20, 16};
        }
        else if (round() == 20) {
          if (corner == 1) objective = {14, 13};
          else if (corner == 2) objective = {14, 16};
          else if (corner == 3) objective = {15, 13};
          else if (corner == 4) objective = {15, 16};
        }
        else if (round() == 25) {
          if (corner == 1) objective = {14, 14};
          else if (corner == 2) objective = {14, 15};
          else if (corner == 3) objective = {15, 14};
          else if (corner == 4) objective = {15, 15};
        }
        else if (round() == 26) {
          if (corner == 1 and cells[15][14] == Stone) objective = {15, 14};
          else if (corner == 1 and cells[14][15] == Stone) objective = {14, 15};
          else if (corner == 2 and cells[15][15] == Stone) objective = {15, 15};
          else if (corner == 2 and cells[14][14] == Stone) objective = {14, 14};
          else if (corner == 3 and cells[14][14]) objective = {14, 14};
          else if (corner == 3 and cells[15][15]) objective = {15, 15};
          else if (corner == 4 and cells[14][15]) objective = {14, 15};
          else if (corner == 4 and cells[15][14]) objective = {14, 15};
          else mode = 0;
        }
        else if (round() == 27) mode = 0;
      }
      else if (strategy == 3) {
        if (round() == 4) {
          if (corner == 1) objective = {5, 1};
          else if (corner == 2) objective = {5, 29};
          else if (corner == 3) objective = {15, 1};
          else if (corner == 4) objective = {15, 29};
        }
        else if (round() >= 17) { // disable avoiding?
          if ((corner == 1 and cells[5][29] != Stone)
              or (corner == 2 and cells[5][1] != Stone)
              or (corner == 3 and cells[15][29] != Stone)
              or (corner == 4 and cells[15][1] != Stone)) {
            mode = 0;
          }
          else {
            if (corner == 1) objective = {5, 29};
            else if (corner == 2) objective = {5, 1};
            else if (corner == 3) objective = {15, 29};
            else if (corner == 4) objective = {15, 1};
          }
        }
        //else if (round() == 45) mode = 0;
      }

    //- Decide action
      if (mode == 0) {

        // IMPROVE: First of all dodge ghost walls or dangerous situations

        Dir attack_dir = do_attack(p);
        if (attack_dir != None) attack(attack_dir);
        else {
          // Generate list of objectives
          priority_queue<obj, vector<obj>, CompareObjectives> objs = get_objs(p.pos);

          //* PRINTING
            if (couts) {
              priority_queue<obj, vector<obj>, CompareObjectives> objs_cpy = objs;
              cout << "Before deleting" << endl;
              while (not objs_cpy.empty()) {
                obj aux = objs_cpy.top(); objs_cpy.pop();
                cout << "priori=" << aux.first.first << " pos=" << aux.first.second << " dir=" << d2c(aux.second.first) << " dis=" << aux.second.second << endl;
              }
            }

          //- Delete dangerous objectives from queue
            // Check if there are stronger poquemons than me
            for (int i = 0; (unsigned)i < p_enemies.size(); ++i) {
              if (p_enemies[i].alive and (p_enemies[i].attack >= p.defense
                  or (p_enemies[i].attack < p.defense and p_enemies[i].defense > p.attack and p_enemies[i].defense >= p.defense))) {
                // Delete all objectives that are closer to this poquemon than me
                // Make a copy of the queue and empty the original, then only insert the ones that are safe
                
                priority_queue<obj, vector<obj>, CompareObjectives> objs_cpy = objs;
                priority_queue<obj, vector<obj>, CompareObjectives> objs_new;
                while (not objs_cpy.empty()) {
                  obj obj_aux = objs_cpy.top(); objs_cpy.pop();

                  if (couts) cout << "Enemy pos=" << p_enemies[i].pos.i << "," << p_enemies[i].pos.j << " Objective pos=" << obj_aux.first.second.i << "," << obj_aux.first.second.j << endl;

                  PDD DirDistEne = go_to(p_enemies[i].pos, Empty, -1, obj_aux.first.second);
                  if (DirDistEne.second != 0 and DirDistEne.second <= obj_aux.second.second) {
                    if (couts) cout << "Deleted because of " << i << " Dist=" << DirDistEne.second << endl;
                  }
                  else {
                    objs_new.push(obj_aux);
                  }
                }
                objs = objs_new;
              }
            }

          //* PRINTING
            if (couts) {
              priority_queue<obj, vector<obj>, CompareObjectives> objs_cpy = objs;
              cout << "After deleting" << endl;
              while (not objs_cpy.empty()) {
                obj aux = objs_cpy.top(); objs_cpy.pop();
                cout << "priori=" << aux.first.first << " pos=" << aux.first.second << " dir=" << d2c(aux.second.first) << " dis=" << aux.second.second << endl;
              }
            }

          if (not objs.empty()) {
            move(objs.top().second.first);
          }
          else {
            Dir aproach_dir = aproach_weaker(p);
            if (aproach_dir != None) move(aproach_dir);
            else {
              Dir rand_dir = random_dir(p.pos);
              //if (rand_dir != None) move(rand_dir);
            }
          }
        }
      }
      else if (mode == 1) {
        move(go_to(p.pos, Empty, -1, objective).first); 
      }
  }

  Dir random_dir(Pos pos) {
    for (int i = 0; (unsigned)i < all_dirs.size(); ++i) {
      if (valid_mov(pos, all_dirs[i])) return all_dirs[i];
    }
    return None;
  }

  Dir aproach_weaker(const Poquemon& p) {
    // If I have nothing to do but I see an enemy weaker than me, go for him
      // Check if there is pokemon right
      // Check if there is pokemon right
      for (int i = 1; p.pos.j + i < cols() and cells[p.pos.i][p.pos.j + i] != Wall; ++i) {
        int e_p_id = cell_id({p.pos.i, p.pos.j + i});
        if (e_p_id != -1) {
          const Poquemon& e_p = poquemon(e_p_id);
          if (p.attack >= e_p.defense or (p.defense > e_p.attack and p.defense > e_p.defense)) {
            return Right;
          }
        }
      }
      // Check if there is pokemon left
      for (int i = 1; p.pos.j - i >= 0 and cells[p.pos.i][p.pos.j - i] != Wall; ++i) {
        int e_p_id = cell_id({p.pos.i, p.pos.j - i});
        if (e_p_id != -1) {
          const Poquemon& e_p = poquemon(e_p_id);
          if (p.attack >= e_p.defense or (p.defense > e_p.attack and p.defense > e_p.defense)) {
            return Left;
          }
        }
      }
      // Check if there is pokemon top
      for (int i = 1; p.pos.i - i >= 0 and cells[p.pos.i - i][p.pos.j] != Wall; ++i) {
        int e_p_id = cell_id({p.pos.i - i, p.pos.j});
        if (e_p_id != -1) {
          const Poquemon& e_p = poquemon(e_p_id);
          if (p.attack >= e_p.defense or (p.defense > e_p.attack and p.defense > e_p.defense)) {
            return Top;
          }
        }
      }
      // Check if there is pokemon bottom
      for (int i = 1; p.pos.i + i < rows() and cells[p.pos.i + i][p.pos.j] != Wall; ++i) {
        int e_p_id = cell_id({p.pos.i + i, p.pos.j});
        if (e_p_id != -1) {
          const Poquemon& e_p = poquemon(e_p_id);
          if (p.attack >= e_p.defense or (p.defense > e_p.attack and p.defense > e_p.defense)) {
            return Bottom;
          }
        }
      }

    return None;
  }

  PDD go_to(Pos p, CType type = Empty, int kind = -1, Pos objective = {-1, -1}) {
    // Position and distance for a certain cell
    typedef pair<Pos, int> PPI;
    // Queue with all cells pending to visit
    queue<PPI> Q;
    //priority_queue<PPI, vector<PPI>, CompareObjectives> Q;
    
    // I save position and tree level of first one
    Q.push(make_pair(p, 0));

    // Vector of already visited cells
    
    // Shall I fill first pos of each PID with false?
    //vector< vector<PID> > cells_visited = vector< vector<PID> >(rows(), vector<PID>(cols(), false));
    vector<vector<PVD> > cells_visited = fixed_walls;

    // I mark current position (first one) as visited, and made no movement to get there
    int row = p.i;
    int col = p.j;
    cells_visited[row][col] = make_pair(true, None);

    while(not Q.empty()) {
      // I get next cell to visit
      PPI Pos_Lev = Q.front(); Q.pop();
      // I obtain position of the cell
      p = Pos_Lev.first;

      // I generate all possible movements
      for (int i = 0; (unsigned)i < all_dirs.size(); ++i) {
        // Check if generated dir is valid
        if (valid_mov(p, all_dirs[i], Pos_Lev.second+1)) {
          // Add movement to p
          Pos p_new = p + all_dirs[i];

          // If cell not visited
          if (not cells_visited[p_new.i][p_new.j].first) {
            // Mark cell as visited
            cells_visited[p_new.i][p_new.j] = make_pair(true, all_dirs[i]);

            // If cell is of type I'm looking for
            if ((type == Empty and (cells[p_new.i][p_new.j] != Empty and cells[p_new.i][p_new.j] != Wall) and not pos_ok(objective))
               or (type != Empty and cells[p_new.i][p_new.j] == type and not pos_ok(objective))
               or (objective.i == p_new.i and objective.j == p_new.j)) {
              if ((kind != -1 and points_value({p_new.i,p_new.j}) == kind) or (kind == -1)) {
                // Go find first move that made possible to arrive to this cell
                Pos p_prev = {p_new.i, p_new.j};
                int i_prev = p_prev.i;
                int j_prev = p_prev.j;
                for (int j = Pos_Lev.second; j > 0; --j) {
                  // Follow each Dir made to get to the bonus cell backwards
                  //p_prev -= cells_visited[i_prev][j_prev].second;
                  if (cells_visited[i_prev][j_prev].second == Top) p_prev += Bottom;
                  else if (cells_visited[i_prev][j_prev].second == Bottom) p_prev += Top;
                  else if (cells_visited[i_prev][j_prev].second == Left) p_prev += Right;
                  else if (cells_visited[i_prev][j_prev].second == Right) p_prev += Left;
                  i_prev = p_prev.i;
                  j_prev = p_prev.j;
                }

                Dir dir_choosen = cells_visited[i_prev][j_prev].second;
                return make_pair(dir_choosen, Pos_Lev.second + 1);

              }
            }
            Q.push(make_pair(p_new, Pos_Lev.second + 1));
          }
        }
      }
    }
    return make_pair(None, 0);
  }

  // Compares 2 positions
  bool pos_eq(Pos p1, Pos p2) {
    return (p1.i == p2.i and p1.j == p2.j);
  }

  Dir do_attack(const Poquemon& p) {
    // There are situations where I can't fight more than one, consider dodging?

    // Queue with enemies I can win with strongest on top
    typedef pair<bool, Dir> POD; // Pair(OneShot?, Direction)
    typedef pair<int, Poquemon> PDP; // Pair(Distance, Poquemon)
    typedef pair<POD,PDP> enemy; // Pair( Pair(OneShot?, Direction), Pair(Distance, Poquemon) )
    struct CompareEnemies {
      bool operator()(enemy const & obj1, enemy const & obj2) {
        return obj1.second.second.attack < obj2.second.second.attack; // (Strength)
      }
    };
    priority_queue<enemy, vector<enemy>, CompareEnemies> enemiesQ;

    //- Atack if I'm safe to

    // Check if there is pokemon right
    for (int i = 1; p.pos.j + i < cols() and cells[p.pos.i][p.pos.j + i] != Wall and i <= p.scope; ++i) {
      int e_p_id = cell_id({p.pos.i, p.pos.j + i});
      if (e_p_id != -1) {
        const Poquemon& e_p = poquemon(e_p_id);
        if (p.attack >= e_p.defense
            or (p.defense > e_p.attack and p.defense > e_p.defense)
            or (i > e_p.scope)) {
          bool one_shot = (p.attack >= e_p.defense);
          enemiesQ.push(make_pair(make_pair(one_shot,Right),make_pair(i,e_p)));
          break;
        }
      }
    }
    // Check if there is pokemon left
    for (int i = 1; p.pos.j - i >= 0 and cells[p.pos.i][p.pos.j - i] != Wall and i <= p.scope; ++i) {
      int e_p_id = cell_id({p.pos.i, p.pos.j - i});
      if (e_p_id != -1) {
        const Poquemon& e_p = poquemon(e_p_id);
        if (p.attack >= e_p.defense
            or (p.defense > e_p.attack and p.defense > e_p.defense)
            or (i > e_p.scope)) {
          bool one_shot = (p.attack >= e_p.defense);
          enemiesQ.push(make_pair(make_pair(one_shot,Left),make_pair(i,e_p)));
          break;
        }
      }
    }
    // Check if there is pokemon top
    for (int i = 1; p.pos.i - i >= 0 and cells[p.pos.i - i][p.pos.j] != Wall and i <= p.scope; ++i) {
      int e_p_id = cell_id({p.pos.i - i, p.pos.j});
      if (e_p_id != -1) {
        const Poquemon& e_p = poquemon(e_p_id);
        if (p.attack >= e_p.defense
            or (p.defense > e_p.attack and p.defense > e_p.defense)
            or (i > e_p.scope)) {
          bool one_shot = (p.attack >= e_p.defense);
          enemiesQ.push(make_pair(make_pair(one_shot,Top),make_pair(i,e_p)));
          break;
        }
      }
    }
    // Check if there is pokemon bottom
    for (int i = 1; p.pos.i + i < rows() and cells[p.pos.i + i][p.pos.j] != Wall and i <= p.scope; ++i) {
      int e_p_id = cell_id({p.pos.i + i, p.pos.j});
      if (e_p_id != -1) {
        const Poquemon& e_p = poquemon(e_p_id);
        if (p.attack >= e_p.defense
            or (p.defense > e_p.attack and p.defense > e_p.defense)
            or (i > e_p.scope)) {
          bool one_shot = (p.attack >= e_p.defense);
          enemiesQ.push(make_pair(make_pair(one_shot,Bottom),make_pair(i,e_p)));
          break;
        }
      }
    }

    // IF: Attack the stronger one I oneshot
    priority_queue<enemy, vector<enemy>, CompareEnemies> enemiesQ_cpy = enemiesQ;
    while (not enemiesQ_cpy.empty()) {
      enemy aux = enemiesQ_cpy.top(); enemiesQ_cpy.pop();
      if (aux.first.first) return aux.first.second;
    }

    // Attack the one that has me in range
    enemiesQ_cpy = enemiesQ;
    while (not enemiesQ_cpy.empty()) {
      enemy aux = enemiesQ_cpy.top(); enemiesQ_cpy.pop();
      if (aux.second.second.scope >= aux.second.first) return aux.first.second;
    }

    // Attack the weaker one
    enemiesQ_cpy = enemiesQ;
    typedef pair<int,Dir> PDD; // Pair(Defense,Direction)
    PDD weaker_one = make_pair(100,None);
    while (not enemiesQ_cpy.empty()) {
      enemy aux = enemiesQ_cpy.top(); enemiesQ_cpy.pop();
      if (aux.second.second.defense < weaker_one.first) weaker_one = make_pair(aux.second.second.defense, aux.first.second);
    }
    return weaker_one.second;

    // IMRPOVE: Consider situations with multiple enemies

    return None;
  }

  priority_queue<obj, vector<obj>, CompareObjectives> get_objs(Pos pos) {
    typedef pair<Pos, int> PPD; // Pair(Position, Distance)
    queue<PPD> Q;
    Q.push(make_pair(pos, 0));

    priority_queue<obj, vector<obj>, CompareObjectives> Q_obj;
    vector<vector<PVD> > visited_cells = fixed_walls;

    visited_cells[pos.i][pos.j] = make_pair(true, None);

    while(not Q.empty()) {
      PPD Pos_Dis = Q.front(); Q.pop();
      Pos n_pos = Pos_Dis.first;

      for (int i = 0; (unsigned)i < all_dirs.size(); ++i) {
        if (valid_mov(n_pos, all_dirs[i], Pos_Dis.second+1)) {
          Pos n_pos2 = n_pos + all_dirs[i];

          if (not visited_cells[n_pos2.i][n_pos2.j].first) {
            visited_cells[n_pos2.i][n_pos2.j] = make_pair(true, all_dirs[i]);

            if (cells[n_pos2.i][n_pos2.j] != Empty) {
              
              // Go find first move that made possible to arrive to this cell
              Pos p_prev = {n_pos2.i, n_pos2.j};
              int i_prev = p_prev.i;
              int j_prev = p_prev.j;
              for (int j = Pos_Dis.second; j > 0; --j) {
                
                // Follow each Dir made to get to the bonus cell backwards
                //p_prev -= visited_cells[i_prev][j_prev].second;
                if (visited_cells[i_prev][j_prev].second == Top) p_prev += Bottom;
                else if (visited_cells[i_prev][j_prev].second == Bottom) p_prev += Top;
                else if (visited_cells[i_prev][j_prev].second == Left) p_prev += Right;
                else if (visited_cells[i_prev][j_prev].second == Right) p_prev += Left;
                i_prev = p_prev.i;
                j_prev = p_prev.j;
              }

              Dir dir_choosen = visited_cells[i_prev][j_prev].second;

              int weight = obj_weight[ctype_id(n_pos2.i, n_pos2.j)] + Pos_Dis.second + 1;
              
              const Poquemon& poq = poquemon(me());
              if (not ((cells[n_pos2.i][n_pos2.j] == Stone and poq.stones == max_stone() and (Pos_Dis.second + 1) > 1)
                     or (cells[n_pos2.i][n_pos2.j] == Scope and poq.scope == max_scope() and (Pos_Dis.second + 1) > 1))) {
                Q_obj.push(make_pair(make_pair(weight,n_pos2),make_pair(dir_choosen, Pos_Dis.second + 1)));
              }
            }
            Q.push(make_pair(n_pos2, Pos_Dis.second + 1));
          }
        }
      }
    }
    return Q_obj;
  }

  bool valid_mov(Pos pos, Dir mov, int in_rounds = 1) {
    Pos n_pos = pos + mov;

    // Check if movement is inside map
    if (not pos_ok(n_pos)) return false;

    // Check if movement is towards wall
    if (fixed_walls[n_pos.i][n_pos.j].first) return false;

    // Check if that movement is towards a ghost wall
    if (ghost_walls[n_pos.i][n_pos.j].first) {
      // If wall is present
      if (ghost_walls[n_pos.i][n_pos.j].second and (in_rounds <= ghost_wall(n_pos))) return false;
      else if (ghost_walls[n_pos.i][n_pos.j].second and (in_rounds > ghost_wall(n_pos))) {
        bool present = ghost_walls[n_pos.i][n_pos.j].second;
        // How many times it will change?
        int rounds_from_first_change = in_rounds-ghost_wall(n_pos);
        if (((int)rounds_from_first_change/wall_change_time())%2 == 0) present = !present;
        if (ghost_walls[n_pos.i][n_pos.j].second == present) return false;
      }
      // If wall is not present
      else if (not ghost_walls[n_pos.i][n_pos.j].second and (in_rounds == ghost_wall(n_pos))) return false;
      else if (not ghost_walls[n_pos.i][n_pos.j].second and (in_rounds > ghost_wall(n_pos))) {
        bool present = ghost_walls[n_pos.i][n_pos.j].second;
        // How many times it will change?
        int rounds_from_first_change = in_rounds-ghost_wall(n_pos);
        if (((int)rounds_from_first_change/wall_change_time())%2 == 0) present = !present;
        if (ghost_walls[n_pos.i][n_pos.j].second != present) return false;
      }
    }

    // Check if that movement is towards an inmediate poquemon
    if (in_rounds == 1 and cell_id(n_pos) != -1) return false;

    return true;
  }

  bool safe_mov(Pos pos, Dir mov, int in_rounds = 1) {

    return true;
  }

  //# Returns index of obj_weight vector depending on CType
  int ctype_id(int i, int j) {
    CType ct = cells[i][j];
    if (ct == Stone) return 0;
    else if (ct == Point) {
      int points = points_value({i,j});
      if (points == 500) return 1;
      else if (points == 400) return 2;
      else if (points == 300) return 3;
      else if (points == 200) return 4;
      else if (points == 100) return 6;
    }
    else if (ct == Attack or ct == Defense) return 5;
    else if (ct == Scope) return 7;
    return 7;
  }

  //# Tells me which maze we are playing on
  int which_maze() {
    vector<vector<CType> > maze0 {{Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall},
    {Wall,Stone,Wall,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Wall,Stone,Wall},
    {Wall,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Wall,Wall,Wall,Empty,Scope,Empty,Wall,Wall,Wall,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Wall},
    {Wall,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Wall,Wall,Empty,Wall,Wall,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Wall},
    {Wall,Point,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Wall},
    {Wall,Empty,Wall,Empty,Empty,Wall,Wall,Wall,Point,Wall,Wall,Wall,Empty,Empty,Empty,Point,Empty,Empty,Empty,Wall,Wall,Wall,Point,Wall,Wall,Wall,Empty,Empty,Wall,Empty,Wall},
    {Wall,Empty,Wall,Empty,Empty,Wall,Defense,Empty,Empty,Empty,Attack,Wall,Empty,Wall,Empty,Wall,Empty,Wall,Empty,Wall,Attack,Empty,Empty,Empty,Defense,Wall,Empty,Empty,Wall,Empty,Wall},
    {Wall,Empty,Empty,Point,Empty,Wall,Wall,Wall,Empty,Wall,Wall,Wall,Point,Wall,Empty,Stone,Empty,Wall,Point,Wall,Wall,Wall,Empty,Wall,Wall,Wall,Empty,Point,Empty,Empty,Wall},
    {Wall,Empty,Wall,Empty,Empty,Wall,Attack,Empty,Empty,Empty,Defense,Wall,Empty,Wall,Empty,Wall,Empty,Wall,Empty,Wall,Defense,Empty,Empty,Empty,Attack,Wall,Empty,Empty,Wall,Empty,Wall},
    {Wall,Empty,Wall,Empty,Empty,Wall,Wall,Wall,Point,Wall,Wall,Wall,Empty,Empty,Empty,Point,Empty,Empty,Empty,Wall,Wall,Wall,Point,Wall,Wall,Wall,Empty,Empty,Wall,Empty,Wall},
    {Wall,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Point,Wall},
    {Wall,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Wall,Wall,Empty,Wall,Wall,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Wall},
    {Wall,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Wall,Wall,Wall,Empty,Scope,Empty,Wall,Wall,Wall,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Wall},
    {Wall,Stone,Wall,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Wall,Stone,Wall},
    {Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall}};
      
    vector<vector<CType> > maze1 {{Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Scope,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Scope,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Point,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Point,Wall},
    {Wall,Empty,Empty,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Scope,Scope,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Stone,Stone,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Stone,Stone,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Scope,Scope,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Empty,Empty,Wall},
    {Wall,Point,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Point,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Scope,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Scope,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall}};
    
    vector<vector<CType> > maze2 {{Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall},
    {Wall,Scope,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Scope,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Stone,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Stone,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Wall,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Wall},
    {Wall,Wall,Wall,Wall,Wall,Wall,Empty,Empty,Empty,Empty,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Empty,Empty,Empty,Empty,Wall,Wall,Wall,Wall,Wall,Wall},
    {Wall,Defense,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Attack,Wall,Attack,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Defense,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Stone,Empty,Empty,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Point,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Point,Empty,Empty,Empty,Stone,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall},
    {Wall,Scope,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Wall,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Empty,Scope,Wall},
    {Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall,Wall}};
    
    if (cells == maze0) return 0;
    else if (cells == maze1) return 1;
    else if (cells == maze2) return 2;
    return -1;
  }
};

/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);