#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <glib.h>

#include <GL/freeglut.h>
#ifdef __APPLE__
  #include <OpenGL/gl.h>
  #include <OpenGL/glu.h>
#else
  #include <GL/gl.h>
  #include <GL/glu.h>
#endif

#define DIST_INF INT_MAX
#define GRID_N 70
#define TICK_RATE 1000.0 / 1000.0

struct vec {
  int x;
  int y;
  int dist;
  int cost;
  int visited;
  int marked;
  struct vec *prev;
};

struct world {
  unsigned long tick;
  struct vec *cur, *start, *target;
  struct vec grid[GRID_N * GRID_N];
  GList *unvisited;
};

struct world W;

/**
 * Return -1 if a is closer than b to the initial position, 0 if they're equal,
 * and 1 otherwise.
 */
gint cmp(gconstpointer a, gconstpointer b) {
  const struct vec *x = a, *y = b;
  return (x->dist < y->dist) ? -1 : (x->dist > y->dist);
}

/**
 * Render the world.
 */
void render() {
  int x, y;
  struct vec *cur;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  glTranslatef(GRID_N * -0.5f, GRID_N * -0.5f, GRID_N * -1.25f);

  glBegin(GL_QUADS);
    for (y = 0; y < GRID_N; y++) {
      for (x = 0; x < GRID_N; x++) {
        cur = &W.grid[y * GRID_N + x];

        if (W.target == cur) {
          glColor3f(1.0f, 1.0f, 1.0f);
        } else if (W.start == cur) {
          glColor3f(0.0f, 1.0f, 0.0f);
        } else if (cur->visited) {
          glColor3f(0.5f, 0.0f, 0.0f);
        } else if (cur->cost == DIST_INF) {
          glColor3f(0.5f, 0.5f, 0.5f);
        } else {
          glColor3f(0.25f, 0.25f, 0.25f);
        }

        glVertex3f((cur->x + 1) * 1.0f, (cur->y    ) * 1.0f, -5.0f);
        glVertex3f((cur->x    ) * 1.0f, (cur->y    ) * 1.0f, -5.0f);
        glVertex3f((cur->x    ) * 1.0f, (cur->y + 1) * 1.0f, -5.0f);
        glVertex3f((cur->x + 1) * 1.0f, (cur->y + 1) * 1.0f, -5.0f);
      }
    }
  glEnd();

  glutSwapBuffers();
}

/**
 * Respond to a window resize.
 */
void resize(int w, int h) {
  float ratio;

  if (h == 0) {
    h = 1;
  }
  ratio = w * 1.0 / h;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(45.0f, ratio, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
}

/**
 * Run the next step of Dijkstra's algorithm.
 */
void update(int tick) {
  struct vec *neighbours[4];
  struct vec *neighbour;
  int i, new_dist;

  printf("update: %d; cur: %d,%d\n", g_list_length(W.unvisited), W.cur->x, W.cur->y);

  neighbours[0] = W.cur->x - 1 >= 0     ? &W.grid[(W.cur->y * GRID_N)       + W.cur->x - 1] : NULL;
  neighbours[1] = W.cur->x + 1 < GRID_N ? &W.grid[(W.cur->y * GRID_N)       + W.cur->x + 1] : NULL;
  neighbours[2] = W.cur->y - 1 >= 0     ? &W.grid[((W.cur->y - 1) * GRID_N) + W.cur->x]     : NULL;
  neighbours[3] = W.cur->y + 1 < GRID_N ? &W.grid[((W.cur->y + 1) * GRID_N) + W.cur->x]     : NULL;

  for (i = 0; i < 4; i++) {
    neighbour = neighbours[i];

    if (neighbour != NULL && !neighbour->marked) {
      if (g_list_find(W.unvisited, neighbour) == NULL) {
        W.unvisited = g_list_prepend(W.unvisited, neighbour); // TODO Replace with an actual set
      }

      neighbour->marked = 1;
      new_dist = W.cur->dist + neighbour->cost;

      if (new_dist < neighbour->dist) {
        neighbour->prev = W.cur;
        neighbour->dist = new_dist;
      }
    }
  }
  W.cur->visited = 1;
  W.cur->marked = 1;

  W.unvisited = g_list_sort(W.unvisited, *cmp);

  if (W.cur == W.target || W.unvisited == NULL) {
    // Stop on found / exhausted all options
    printf(W.cur == W.target ? "path found\n" : "no path found\n");

    glutLeaveMainLoop();
  } else {
    W.cur = W.unvisited->data;
    W.unvisited = g_list_remove(W.unvisited, W.unvisited->data);

    glutPostRedisplay();
    glutTimerFunc(TICK_RATE, update, tick + 1);
  }
}

/**
 * Initalize the world grid and select the initial and end nodes.
 */
void setup_world(struct world *world) {
  struct vec *v;
  int x, y;

  for (y = 0; y < GRID_N; y++) {
    for (x = 0; x < GRID_N; x++) {
      v = &W.grid[y * GRID_N + x];
      v->x = x;
      v->y = y;
      v->cost = rand() % 100 > 65 ? DIST_INF : 1;
      v->visited = 0;
      v->marked = 0;
      v->dist = DIST_INF;
      v->prev = NULL;
    }
  }

  W.start = &W.grid[rand() % (GRID_N * GRID_N)];
  W.target = &W.grid[rand() % (GRID_N * GRID_N)];

  W.unvisited = NULL;
  W.cur = W.start;
  W.cur->cost = 0;
  W.cur->dist = 0;
}

/**
 * Initialize OpenGL.
 */
int setup_renderer(int *argc, char *argv[]) {
  glutInit(argc, argv);
  glutInitWindowSize(400, 400);
  glutInitWindowPosition(-1, -1);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

  return glutCreateWindow("Search");
}

/**
 * Called on exit.
 */
void cleanup() {
  g_list_free(W.unvisited);
}

int main(int argc, char *argv[]) {
  // Set initial state
  srand(time(NULL));

  setup_world(&W);
  setup_renderer(&argc, argv);

  glutDisplayFunc(render);
  glutReshapeFunc(resize);
  glutTimerFunc(TICK_RATE, update, 0);

  glEnable(GL_DEPTH_TEST);
  atexit(cleanup);

  glutMainLoop();

  return 0;
}
