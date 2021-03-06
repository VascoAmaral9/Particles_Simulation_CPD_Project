// main.cpp
#include "physics.h"

int main (int argc, char* argv[]) {
  int i, j;
  int a;
  int seed, grid_sz, part_no, steps;
  double totalM;

  if (argc!=5){
    std::cout << "Incorrect number of arguments." << std::endl;
    exit(1);
  }

  std::istringstream iss( argv[1] );
  if (iss >> seed){
      std::cout << "Seed nº:" << seed << std::endl;
  }

  std::istringstream iss1( argv[2] );
  if (iss1 >> grid_sz){
      std::cout << "Grid size:" << grid_sz << std::endl;
  }

  std::istringstream iss2( argv[3] );
  if (iss2 >> part_no){
      std::cout << "Nº of particles:"<< part_no << std::endl;
  }

  std::istringstream iss3( argv[4] );
  if (iss3 >> steps){
      std::cout << "Nº of steps:" << steps << std::endl;
  }

  //Start both structures
  Grid **grid = new Grid*[grid_sz];
  for(i=0;i<grid_sz;i++){
    grid[i] = new Grid[grid_sz];
  }
  Particle *par = new Particle[part_no];

  init_grid(grid_sz, grid);
  totalM = init_particles(seed,grid_sz,part_no,par, grid);

  /* ciclo baseado no numero de steps */
  for(i=0; i<steps; i++){
    update_center_all(grid_sz, grid, par);
    clear_grid(grid_sz, grid);
    #pragma omp parallel private(i)
        {
          #pragma omp for
          /* 2.1. PROCESS ELEMENTS */
          for(j=0; j<part_no; j++){
              move_particle(grid_sz, &par[j], grid, j);
          }
        }
    swap_grid_Ms(grid_sz, grid);
  }

  printf("%f %f\n", par[0].getX(), par[0].getY());

  overall_center(par, part_no, totalM);

  return 0;
}
