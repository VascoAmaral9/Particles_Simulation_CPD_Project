//grid.c
#include "grid.h"

Grid **init_grid(long size){
    long i, j;

    Grid **grid = NULL;

    grid = (Grid**) malloc(sizeof(Grid*) * size);

    for(i=0;i<size;i++){
        grid[i] = (Grid*) malloc(sizeof(Grid) * size);
        for(j=0;j<size;j++){
            grid[i][j].center.x = 0;
            grid[i][j].center.y = 0;
            grid[i][j].M = 0;
            grid[i][j].Mnext = 0;
        }
    }

    return grid;
}

/* Função para fazer o update geral de todos os centros de massa*/
void update_center_local (long long part_no, long size, Grid **grid, Particle *par){
  long long i;
  int Gx, Gy;
  double new_x, new_y;

  #pragma omp parallel
  {
    #pragma omp for private (i,Gx,Gy,new_x,new_y)
    for(i=0;i<part_no;i++){
      Gx = floor(par[i].pos.x * size);
      if(Gx == size)
        Gx = size - 1;
      Gy = floor(par[i].pos.y * size);
      if(Gy == size)
        Gy = size - 1;


      new_x = (par[i].pos.x * par[i].m)/grid[Gx][Gy].M;
      #pragma omp atomic
      grid[Gx][Gy].center.x += new_x;

      new_y = (par[i].pos.y * par[i].m)/grid[Gx][Gy].M;
      #pragma omp atomic
      grid[Gx][Gy].center.y += new_y;
    }
  }
}


/* Function to calculate local overall center of mass and saves it in grid[0][0] position*/
void overall_center_local(Particle *par, Grid **grid, long long part_no, double totalM){
  long long i;
  double x=0, y=0;
  double x_aux, y_aux;

  #pragma omp parallel
  {
    #pragma omp for private (i, x_aux, y_aux) reduction (+:x,y)
    for(i=0; i<part_no; i++){
      x += (par[i].pos.x * par[i].m)/totalM;
      y += (par[i].pos.y * par[i].m)/totalM;
    }
  }

  grid[0][0].center.x = x;
  grid[0][0].center.y = y;
}

void broadcast_mass_centers(Grid **grid, int p_rank, int n_pr, long grid_sz){
    //double buffers[n_pr][2*grid_sz*grid_sz];
    double ***buffers = NULL;
    int d;
    long i, j, l = 0;
    double new_x = 0, new_y = 0;
    int n_buffers, curr_buffer = 0;

    n_buffers = ceil((2*grid_sz*grid_sz)/(GRIDBUFFER));
    //double buffers[n_pr][n_buffers][GRIDBUFFER];

    buffers = (double***) malloc(sizeof(double**) * n_pr);
    for(i = 0; i < n_pr; i++){
      buffers[i] = (double**) malloc(sizeof(double*) * n_buffers);
      for(j=0; j<n_buffers; j++)
          buffers[i][j] = (double*) malloc(sizeof(double) * GRIDBUFFER);
    }


    for(i = 0; i < grid_sz; i++){
      for(j = 0; j < grid_sz; j++){
         if(l == GRIDBUFFER/2){
           curr_buffer++;
           l = 0;
         }
         buffers[p_rank][curr_buffer][2*l] = grid[i][j].center.x;
         buffers[p_rank][curr_buffer][(2*l)+1] = grid[i][j].center.y;
         l++;
      }
    }

    for(curr_buffer = 0; curr_buffer < n_buffers; curr_buffer++)
      for(d = 0; d < n_pr; d++)
        MPI_Bcast(buffers[d][curr_buffer], GRIDBUFFER, MPI_DOUBLE, d, MPI_COMM_WORLD);

    l = 0;
    curr_buffer = 0;
    for(i = 0; i < grid_sz; i++){
      for(j = 0; j < grid_sz; j++){
        if(l == GRIDBUFFER/2){
          curr_buffer++;
          l = 0;
        }
        for(d = 0; d < n_pr; d++){
          new_x += buffers[d][curr_buffer][2*l];
          new_y += buffers[d][curr_buffer][(2*l)+1];
        }
        grid[i][j].center.x = new_x;
        grid[i][j].center.y = new_y;
        grid[i][j].Mnext = 0;
        new_x = 0;
        new_y = 0;
        l++;
      }
    }

    #pragma omp parallel
    {
      #pragma omp for private (i,j)
      for(i = 0; i < n_pr; i++){
        for(j=0; j < n_buffers; j++){
          free(buffers[i][j]);
        }
        free(buffers[i]);
      }
    }
    free(buffers);
}

void broadcast_mass(Grid **grid, int p_rank, int n_pr, long grid_sz){
    //double buffers[n_pr][grid_sz*grid_sz];
    double ***buffers = NULL;
    int d;
    long i, j, k, l = 0;
    double new_m = 0;
    int n_buffers, curr_buffer = 0;

    n_buffers = ceil((grid_sz*grid_sz)/GRIDBUFFER);
    //double buffers[n_pr][n_buffers][GRIDBUFFER];

    buffers = (double***) malloc(sizeof(double**) * n_pr);
    for(i = 0; i < n_pr; i++){
      buffers[i] = (double**) malloc(sizeof(double*) * n_buffers);
      for(j=0; j<n_buffers; j++)
          buffers[i][j] = (double*) malloc(sizeof(double) * GRIDBUFFER);
    }

    for(i = 0; i < grid_sz; i++){
      for(j = 0; j < grid_sz; j++){
         if(l == GRIDBUFFER){
           curr_buffer++;
           l = 0;
         }
         buffers[p_rank][curr_buffer][l] = grid[i][j].Mnext;
         l++;
      }
    }

    for(curr_buffer = 0; curr_buffer < n_buffers; curr_buffer++)
      for(d = 0; d < n_pr; d++)
        MPI_Bcast(buffers[d][curr_buffer], GRIDBUFFER, MPI_DOUBLE, d, MPI_COMM_WORLD);

    l = 0;
    curr_buffer = 0;
    for(i = 0; i < grid_sz; i++){
      for(j = 0; j < grid_sz; j++){
        if(l == GRIDBUFFER){
          curr_buffer++;
          l = 0;
        }
        for(d = 0; d < n_pr; d++)
          new_m += buffers[d][curr_buffer][l];

        grid[i][j].M = new_m;
        grid[i][j].center.x = 0;
        grid[i][j].center.y = 0;
        new_m = 0;
        l++;
      }
    }

    #pragma omp parallel
    {
      #pragma omp for private (i,j)
      for(i = 0; i < n_pr; i++){
        for(j=0; j < n_buffers; j++){
          free(buffers[i][j]);
        }
        free(buffers[i]);
      }
    }
    free(buffers);
}

void broadcast_overall_center(Grid **grid, int p_rank, int n_pr, double *x, double *y){
    double buffers[n_pr][2];
    int d;
    *x = 0;
    *y = 0;

    buffers[p_rank][0] = grid[0][0].center.x;
    buffers[p_rank][1] = grid[0][0].center.y;

    for(d = 0; d < n_pr; d++)
        MPI_Bcast(buffers[d], 2, MPI_DOUBLE, d, MPI_COMM_WORLD);


    for(d = 0; d < n_pr; d++){
        *x += buffers[d][0];
        *y += buffers[d][1];
    }
}

void broadcast_totalM(int p_rank, int n_pr, double *totalM){
    double buffer[1];

    if(!p_rank){
        buffer[0] = *totalM;
        MPI_Bcast(buffer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    } else{
        MPI_Bcast(buffer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        *totalM = buffer[0];
    }
}

void free_all(Particle *par, Grid  **grid, long grid_sz, int *par_block){
  long i, j;

  free(par);
  #pragma omp parallel
  {
    #pragma omp for private (i)
    for(i = 0; i < grid_sz; i++)
      free(grid[i]);
  }
  free(grid);
  free(par_block);
}
