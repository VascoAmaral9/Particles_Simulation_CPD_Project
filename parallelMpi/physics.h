// physics.h
#include "grid.h"

void divide_par(int n_pr, int part_no, int rem, int *par_block);
int task_owner(int part_no, int n_pr, int rem, int par);
double init_particles(long seed, long ncside, long long n_part, Particle *par, Grid **grid, int *par_block);
double get_distance(Vector a, Vector b);
Vector get_force(Particle par, Grid grid, int add_x, int add_y);
Vector sum_forces(Vector *f);
void move_particle(long grid_sz, Particle *par, Grid **grid, long long id);


// showlist(std::list <int> g);