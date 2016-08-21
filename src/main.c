#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <glib.h>

#define DIST_INF INT_MAX
#define GRID_N 50

struct vec {
  int x;
  int y;
  int dist;
  int cost;
  int visited;
  int marked;
  struct vec *prev;
};

gint cmp(gconstpointer a, gconstpointer b) {
    const struct vec *x = a, *y = b;
    return (x->dist < y->dist) ? -1 : (x->dist > y->dist);
}

int main() {
  GList *unvisited;
  struct vec grid[GRID_N * GRID_N];
  struct vec *v, *start, *target, *cur, *neighbour;
  struct vec *neighbours[4];
  int x, y, i, new_dist;

  srand(time(NULL));

  // Initialize the grid
  for (y = 0; y < GRID_N; y++) {
    for (x = 0; x < GRID_N; x++) {
      v = &grid[y * GRID_N + x];
      v->x = x;
      v->y = y;
      v->cost = 1;
      v->visited = 0;
      v->marked = 0;
      v->dist = DIST_INF;
      v->prev = NULL;
    }
  }

  // Set initial state
  start = &grid[rand() % (GRID_N * GRID_N)];
  target = &grid[rand() % (GRID_N * GRID_N)];
  unvisited = NULL;
  cur = start;
  cur->cost = 0;
  cur->dist = 0;

  while (1) {
    neighbours[0] = cur->x - 1 >= 0     ? &grid[(cur->y * GRID_N)       + cur->x - 1] : NULL;
    neighbours[1] = cur->x + 1 < GRID_N ? &grid[(cur->y * GRID_N)       + cur->x + 1] : NULL;
    neighbours[2] = cur->y - 1 >= 0     ? &grid[((cur->y - 1) * GRID_N) + cur->x]     : NULL;
    neighbours[3] = cur->y + 1 < GRID_N ? &grid[((cur->y + 1) * GRID_N) + cur->x]     : NULL;

    for (i = 0; i < 4; i++) {
      neighbour = neighbours[i];

      if (neighbour != NULL && !neighbour->marked) {
        neighbour->marked = 1;

        unvisited = g_list_prepend(unvisited, neighbour);
        new_dist = cur->dist + neighbour->cost;

        if (new_dist < neighbour->dist) {
          neighbour->prev = cur;
          neighbour->dist = new_dist;
        }
      }
      unvisited = g_list_sort(unvisited, *cmp);
    }
    cur->visited = 1;
    cur->marked = 1;

    // Print a map
    // system("clear");
    printf("cur=%d,%d; target=%d,%d, unvisited=%d\n", cur->x, cur->y, target->x, target->y, g_list_length(unvisited));
    for (y = 0; y < GRID_N; y++) {
      for (x = 0; x < GRID_N; x++) {
        v = &grid[y * GRID_N + x];
        if (v == target) {
            printf("X");
        } else if (v->dist == DIST_INF) {
            printf("0");
        } else {
            printf("%d", v->dist);
        }
      }
      printf("\n");
    }
    usleep(10000);

    // Stop on found / exhausted all options
    if (cur == target || unvisited == NULL) {
      printf(cur == target ? "path found\n" : "no path found\n");
      break;
    }

    cur = unvisited->data;
    unvisited = g_list_remove(unvisited, unvisited->data);
  }

  g_list_free(unvisited);
}
