// main.cpp
#include "grid.h"

int main (int argc, char* argv[]) {
  int i;
  int a;
  int seed, grid_sz, part_no, steps;

  if (argc!=5){
    std::cout << "Incorrect number of arguments." << std::endl;
    exit(1);
  }
  std::istringstream iss( argv[1] );
  if (iss >> seed)
  {
      std::cout << "Seed nº:" << seed << std::endl;
  }
  std::istringstream iss1( argv[2] );
  if (iss1 >> grid_sz)
  {
      std::cout << "Grid size:" << grid_sz << std::endl;
  }
  std::istringstream iss2( argv[3] );
  if (iss2 >> part_no)
  {
      std::cout << "Nº of particles:"<< part_no << std::endl;
  }
  std::istringstream iss3( argv[4] );
  if (iss3 >> steps)
  {
      std::cout << "Nº of steps:" << steps << std::endl;
  }

  Particle par[part_no];
  init_particles(seed,grid_sz,part_no,par);
  /*
  for(i=0; i<part_no; i++){
    par[i].setAll(0.23, 0.45, 0.67, 0.78, 23);
    std::cout << "X: " << par[i].getX() << std::endl;
    std::cout << "Y: " << par[i].getY() << std::endl;
    std::cout << "VX: " << par[i].getVX() << std::endl;
    std::cout << "VY: " << par[i].getVY() << std::endl;
    std::cout << "M: " << par[i].getM() << std::endl;
    std::cout << "\n";
  }

*/

  return 0;
}