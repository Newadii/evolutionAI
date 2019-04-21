#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

#define GENERATION_SIZE 100
#define GENERATION_PRINT_JUMP 100

unsigned int x_dim, y_dim;
static char **map;
int max_fitness;

typedef struct ROCK_POS {
    int x;
    int y;
    struct ROCK_POS *next;
} rock_pos;
rock_pos *first;
int rock_count;

typedef struct UNIT_GENE {
    unsigned int fitness;
    char *in_pos; //in posiotion
    char *collision; //collision direction - 1: up/right 0: down/left
} unit_gene;

void conf_init(int argc, char **argv)
{
    if (argc <= 1)
    {
        printf("no map configuration file!");
        exit(EXIT_FAILURE);
    }
    FILE *fp = fopen(argv[1], "r");
    if (!fp)
    {
        printf("can't open configuration file!");
        exit(EXIT_FAILURE);
    }
    rock_pos *rp, *tmp = NULL;
    rp = first = malloc(sizeof(rock_pos));
    rp->next = NULL;

    fscanf(fp, "%d%d", &x_dim, &y_dim);

    while (fscanf(fp, "%d%d", &(rp->x), &(rp->y)) != -1)
    {
        rock_count++;
        tmp = rp;
        rp->next = malloc(sizeof(rock_pos));
        rp = rp->next;
        rp->next = NULL;
    }
    free(rp);
    if (tmp)
        tmp->next = NULL;
    fclose(fp);
    max_fitness = (x_dim * y_dim) - rock_count;
}

void make_map()
{
    if (!map)
    {
        map = malloc((x_dim+1) * sizeof(char *));
        for (int i=0; i<x_dim+1; i++)
        {
            map[i] = malloc((y_dim+1) * sizeof(char));
        }
    }
    for (int i=0; i<x_dim; i++)
    {
        memset(map[i], 0, y_dim);
        map[i][y_dim] = -2;
    }
    memset(map[x_dim], -2, y_dim);
    rock_pos *rp = first;
    int i=1;
    while (rp)
    {

        map[rp->x][rp->y] = i;
        rp = rp->next;
        i++;
    }
}

int crawl_map(int x, int y, int dir, int axys, unit_gene *g, int *flag)
{
    int fitness=0;
    int tmp_dir;
    while (1)
    {
        if (x < 0 || y < 0 || map[x][y] == -2)
        {
            break;
        } else if (map[x][y] == 0)
        {
            fitness++;
            map[x][y] = -1;
            if (axys == 1)
                y += dir;
            else
                x += dir;
        } else if (map[x][y] > 0)
        {
            tmp_dir = g->collision[map[x][y]-1] == 1 ? 1 : -1;
            if (axys == 1)
                y -= dir;
            else
                x -= dir;
            dir = tmp_dir;
            axys ^= 1;
            if (axys == 1)
                y -= dir;
            else
                x -= dir;
        } else if (map[x][y] == -1)
        {
            if (axys == 1)
                y -= dir;
            else
                x -= dir;
            if (x < 0 || y < 0 || map[x][y] == -2)
                break;
            else if (map[x+1][y] > -1)
                x++;
            else if (x>1 && map[x-1][y] > -1)
                x--;
            else if (map[x][y+1] > -1)
                y++;
            else if (y>0 && map[x][y-1] > -1)
                y--;
            else
            {
                *flag = 1;
                break;
            }
        } else
        {
            printf("something went wrong!\n");
            exit(EXIT_FAILURE);
        }
    }
    return fitness;
}

//returns fitness
void test_gene(unit_gene *g)
{
    int flag = 0;
    unsigned int fitness = 0;
    int axys, dir, tmp_dir, x, y;
    make_map();
    for (int i=0; i<x_dim; i++)
    {
        x = i;
        y = g->in_pos[i] == 1 ? 0 : y_dim -1;
        dir = g->in_pos[i] == 1 ? 1 : -1;
        axys = 1;
        fitness += crawl_map(x, y, dir, axys, g, &flag);
        if (flag)
        {
            g->fitness = fitness;
            return;
        }
    }

    for (int i=0; i<y_dim; i++)
    {
        x = g->in_pos[i + x_dim] == 1 ? 0 : x_dim -1;
        y = i;
        dir = g->in_pos[i + x_dim] == 1 ? 1 : -1;
        axys = 0;
        fitness += crawl_map(x, y, dir, axys, g, &flag);
        if (flag)
        {
            g->fitness = fitness;
            return;
        }
    }
    g->fitness = fitness;
}

void delete_unit(unit_gene *g)
{
    free(g->collision);
    free(g->in_pos);
    free(g);
}

unit_gene **create_random_generation()
{
    unit_gene **units = malloc(GENERATION_SIZE * sizeof(unit_gene *));
    for (int i=0; i<GENERATION_SIZE; i++)
    {
        units[i] = malloc(sizeof(unit_gene));
        units[i]->in_pos = malloc((x_dim + y_dim) * sizeof(char));
        units[i]->collision = malloc(rock_count * sizeof(char));
        for (int k=0; k < rock_count; k++)
            units[i]->collision[k] = (char)(rand() & 1);
        for (int k=0; k < x_dim + y_dim; k++)
            units[i]->in_pos[k] = (char)(rand() & 1);
    }
    return units;
}

unit_gene *mutate_units(unit_gene *g1, unit_gene *g2)
{
    time_t t;

    unit_gene *new_g;
    if (g1->fitness < g2->fitness)
    {
        new_g = g1;
        g1 = g2;
        g2 = new_g;
    }
    new_g = malloc(sizeof(unit_gene));
    new_g->in_pos = malloc((x_dim + y_dim) * sizeof(char));
    new_g->collision = malloc(rock_count * sizeof(char));
    for (int i=0; i < rock_count; i++)
    {
        if (rand() % 3)
            new_g->collision[i] = g2->collision[i];
        else
            new_g->collision[i] = g1->collision[i];
    }
    for (int i=0; i < y_dim + x_dim; i++)
    {
        if (rand() % 3)
            new_g->collision[i] = g2->collision[i];
        else
            new_g->collision[i] = g1->collision[i];
    }
    return new_g;
}

void create_new_generation(unit_gene **units)
{
    for (int i=0; i < GENERATION_SIZE/2; i++)
    {
        if (units[i]->fitness > units[GENERATION_SIZE/2 + i]->fitness && rand() % 913)
        {
            delete_unit(units[GENERATION_SIZE/2 + i]);
        } else
        {
            delete_unit(units[i]);
            units[i] = units[GENERATION_SIZE/2 + i];
        }
    }
    for (int i=0; i<GENERATION_SIZE/4; i++)
    {
        units[GENERATION_SIZE/2 + i] = mutate_units(units[i], units[GENERATION_SIZE/4 + i]);
    }
    int index;
    for (int i=0; i<GENERATION_SIZE/4; i++)
    {
        index = GENERATION_SIZE-1 - i;
        units[index] = malloc(sizeof(unit_gene));
        units[index]->in_pos = malloc((x_dim + y_dim) * sizeof(char));
        units[index]->collision = malloc(rock_count * sizeof(char));
        for (int k=0; k < rock_count; k++)
            units[index]->collision[k] = (char)(rand() & 1);
        for (int k=0; k < x_dim + y_dim; k++)
            units[index]->in_pos[k] = (char)(rand() & 1);
    }
}

void print_map()
{
    for (int i=0; i<x_dim; i++)
    {
        for (int k=0; k<y_dim; k++)
        {
            printf("%c ", map[i][k] + '0');
        }
        printf("\n");
    }
}

int main(int argc, char *argv[], char *envp[])
{
    time_t t;
    srand((unsigned) time(&t));
    if (argc < 3)
    {
        printf("please enter number of generations!\n");
        return EXIT_FAILURE;
    }
    long gen_count = strtol(argv[2], NULL, 10);

    conf_init(argc, argv);
    unit_gene **units = create_random_generation();

    int best_fitness;
    for (int a=0; a<gen_count; a++)
    {
        best_fitness = 0;
        for (int i = 0; i < GENERATION_SIZE; i++)
        {
            test_gene(units[i]);
//          printf("gen%d, unit%d, fitness: %d\n", a, i, units[i]->fitness);
            if (units[i]->fitness > best_fitness)
            {
                best_fitness = units[i]->fitness;
            }
        }
        if (a%GENERATION_PRINT_JUMP == 0)
        {
            printf("gen%d, best fitness: %d\n", a, best_fitness);
        }
        if (best_fitness == max_fitness)
        {
            printf("max fitness reached!\n");
            break;
        }
        create_new_generation(units);
    }
    for (int i=0; i<GENERATION_SIZE; i++)
        delete_unit(units[i]);
    free(units);
    for (int i=0; i<x_dim+1; i++)
        free(map[i]);
    free(map);
    return 0;
}