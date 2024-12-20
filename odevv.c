#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INVENTORY 5
#define MAX_DESC 256

// Structures
typedef struct Item {
    char *name;
    char *description;
} Item;

typedef struct Creature {
    char *name;
    char *description;
    int health;
    int strength;
} Creature;

typedef struct Room {
    char *name;
    char *description;
    Item *item;
    Item *extra_item; // Space for a second item
    Creature *creature;
    struct Room *up;
    struct Room *down;
    struct Room *left;
    struct Room *right;
} Room;

typedef struct Player {
    int health;
    int strength;
    int inventory_count;
    int max_health;
    int max_strength;
    int max_inventory;
    Item *inventory[MAX_INVENTORY];
    Room *current_room;
} Player;

// Global Variables
Player player;
Room *rooms = NULL;

// Function Prototypes
void init_game();
void cleanup_game();
void move(char *direction);
void look();
void pickup(char *item_name);
void attack();
void save_game(const char *filepath);
void load_game(const char *filepath);
void inventory();
void show_options();
void command_loop();

// Game Initialization and Cleanup Functions
void init_game() {
    player.max_health = 100;
    player.max_strength = 15;
    player.max_inventory = MAX_INVENTORY;
    player.health = player.max_health;
    player.strength = player.max_strength;
    player.inventory_count = 0;
    player.current_room = NULL;

    // Create and connect rooms
    Room *grid[3][3];

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            grid[i][j] = malloc(sizeof(Room));
            char *name = malloc(MAX_DESC);
            char *desc = malloc(MAX_DESC);
            snprintf(name, MAX_DESC, "%d. Room", i * 3 + j + 1);
            snprintf(desc, MAX_DESC, "This is %d. Room.", i * 3 + j + 1);
            grid[i][j]->name = name;
            grid[i][j]->description = desc;
            grid[i][j]->item = NULL;
            grid[i][j]->extra_item = NULL;
            grid[i][j]->creature = NULL;
            grid[i][j]->up = grid[i][j]->down = grid[i][j]->left = grid[i][j]->right = NULL;
        }
    }

    // Connect rooms
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i > 0) grid[i][j]->up = grid[i - 1][j];
            if (i < 2) grid[i][j]->down = grid[i + 1][j];
            if (j > 0) grid[i][j]->left = grid[i][j - 1];
            if (j < 2) grid[i][j]->right = grid[i][j + 1];
        }
    }

    // Starting room
    player.current_room = grid[1][1];
    rooms = grid[0][0];

    // Add content to rooms
    char *items[] = {"Sword", "Shield", "Potion", "Key", "Jewel", "Map", "Torch", "Armor"};
    char *item_descriptions[] = {
        "A sharp sword, can be used against enemies.",
        "A sturdy shield, provides extra protection.",
        "A magical potion, restores your health.",
        "A rusty key, may open an old door.",
        "A shiny jewel, very valuable.",
        "An old map, reveals hidden places.",
        "A torch, lights your way in the dark.",
        "A sturdy armor, protects against enemy blows."
    };

    char *extra_items[] = {"Talisman", "Medallion", "Golden Cup", "Diamond"};
    char *extra_item_descriptions[] = {
        "A mysterious talisman, holds special powers.",
        "An ancient medallion, from old times.",
        "A golden cup, very valuable.",
        "A shiny diamond, priceless."
    };

    char *creatures[] = {"Goblin", "Orc", "Troll", "Dragon", "Wolf", "Beast", "Vampire", "Ghost"};
    char *creature_descriptions[] = {
        "A sneaky goblin, fast but weak.",
        "An experienced orc in combat.",
        "A strong troll, instills fear in enemies.",
        "A legendary dragon, very dangerous.",
        "A fast and strong wolf.",
        "A terrifying beast, approaches silently.",
        "A blood-sucking vampire, very agile.",
        "A ghost, can pass through walls."
    };

    int item_index = 0;
    int extra_item_index = 0;
    int creature_index = 0;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i][j] == player.current_room) continue; // Starting room

            grid[i][j]->item = malloc(sizeof(Item));
            grid[i][j]->item->name = strdup(items[item_index]);
            grid[i][j]->item->description = strdup(item_descriptions[item_index]);

            if (i % 2 == 0 && j % 2 == 0) { // Add extra item to 4 rooms
                grid[i][j]->extra_item = malloc(sizeof(Item));
                grid[i][j]->extra_item->name = strdup(extra_items[extra_item_index]);
                grid[i][j]->extra_item->description = strdup(extra_item_descriptions[extra_item_index]);
                extra_item_index = (extra_item_index + 1) % 4;
            }

            if (!(i == 1 && j == 0) && !(i == 1 && j == 2)) { // No creature in 4th and 6th rooms
                grid[i][j]->creature = malloc(sizeof(Creature));
                grid[i][j]->creature->name = strdup(creatures[creature_index]);
                grid[i][j]->creature->description = strdup(creature_descriptions[creature_index]);
                grid[i][j]->creature->health = 50 + creature_index * 10;
                grid[i][j]->creature->strength = 10 + creature_index * 5;
                creature_index = (creature_index + 1) % 8;
            }

            item_index = (item_index + 1) % 8;
        }
    }
}

void cleanup_game() {
    Room *current = rooms;
    Room *visited[9] = {NULL};
    int count = 0;

    while (current && count < 9) {
        visited[count++] = current;

        if (current->name) free(current->name);
        if (current->description) free(current->description);
        if (current->item) {
            free(current->item->name);
            free(current->item->description);
            free(current->item);
        }
        if (current->extra_item) {
            free(current->extra_item->name);
            free(current->extra_item->description);
            free(current->extra_item);
        }
        if (current->creature) {
            free(current->creature->name);
            free(current->creature->description);
            free(current->creature);
        }

        Room *next = NULL;
        if (current->right) {
            next = current->right;
        } else if (current->down) {
            next = current->down;
        }
        free(current);
        current = next;
    }

    for (int i = 0; i < player.inventory_count; i++) {
        free(player.inventory[i]->name);
        free(player.inventory[i]->description);
        free(player.inventory[i]);
    }
}

void look() {
    printf("%s - %s\n", player.current_room->name, player.current_room->description);
    if (player.current_room->item) {
        printf("This room contains a %s: %s\n", player.current_room->item->name, player.current_room->item->description);
    }
    if (player.current_room->extra_item) {
        printf("This room also contains a %s: %s\n", player.current_room->extra_item->name, player.current_room->extra_item->description);
    }
    if (player.current_room->creature) {
        printf("There is a %s here: %s\n", player.current_room->creature->name, player.current_room->creature->description);
    }
}

void move(char *direction) {
    Room *next_room = NULL;
    if (strcmp(direction, "up") == 0) next_room = player.current_room->up;
    else if (strcmp(direction, "down") == 0) next_room = player.current_room->down;
    else if (strcmp(direction, "left") == 0) next_room = player.current_room->left;
    else if (strcmp(direction, "right") == 0) next_room = player.current_room->right;

    if (next_room) {
        player.current_room = next_room;
        printf("You entered a new room: %s\n", player.current_room->name);
    } else {
        printf("You cannot go in this direction.\n");
    }
}

void pickup(char *item_name) {
    if (player.current_room->item && strcmp(player.current_room->item->name, item_name) == 0) {
        if (player.inventory_count < player.max_inventory) {
            player.inventory[player.inventory_count] = player.current_room->item;
            player.current_room->item = NULL;
            printf("%s has been added to your inventory.\n", player.inventory[player.inventory_count]->name);
            player.inventory_count++;
            return;
        } else {
            printf("Your inventory is full.\n");
            return;
        }
    }

    if (player.current_room->extra_item && strcmp(player.current_room->extra_item->name, item_name) == 0) {
        if (player.inventory_count < player.max_inventory) {
            player.inventory[player.inventory_count] = player.current_room->extra_item;
            player.current_room->extra_item = NULL;
            printf("%s has been added to your inventory.\n", player.inventory[player.inventory_count]->name);
            player.inventory_count++;
            return;
        } else {
            printf("Your inventory is full.\n");
            return;
        }
    }

    printf("%s could not be found in this room.\n", item_name);
}

void inventory() {
    if (player.inventory_count == 0) {
        printf("Your inventory is empty.\n");
        return;
    }
    printf("Your inventory:\n");
    for (int i = 0; i < player.inventory_count; i++) {
        printf("- %s: %s\n", player.inventory[i]->name, player.inventory[i]->description);
    }
}

void attack() {
    if (!player.current_room->creature) {
        printf("There is no creature to fight here.\n");
        return;
    }

    Creature *creature = player.current_room->creature;
    while (player.health > 0 && creature->health > 0) {
        creature->health -= player.strength;
        printf("You attacked the %s. Remaining health: %d\n", creature->name, creature->health);

        if (creature->health > 0) {
            player.health -= creature->strength;
            printf("The %s attacked you. Your remaining health: %d\n", creature->name, player.health);
        }
    }

    if (player.health <= 0) {
        printf("You lost!\n");
        exit(0);
    } else {
        printf("The %s has been defeated.\n", creature->name);
        creature->health = 0; // Mark the creature as dead
    }
}

void save_game(const char *filepath) {
    FILE *file = fopen(filepath, "w");
    if (!file) {
        printf("Save file could not be created.\n");
        return;
    }

    fprintf(file, "%d %d\n", player.health, player.inventory_count);
    for (int i = 0; i < player.inventory_count; i++) {
        fprintf(file, "%s\n", player.inventory[i]->name);
    }

    Room *current = rooms;
    while (current) {
        if (current->creature) {
            fprintf(file, "%s %d\n", current->name, current->creature->health);
        } else {
            fprintf(file, "%s NONE\n", current->name);
        }
        current = current->right ? current->right : current->down;
    }

    fclose(file);
    printf("Game saved: %s\n", filepath);
}

void load_game(const char *filepath) {
    FILE *file = fopen(filepath, "r");
    if (!file) {
        printf("Save file could not be loaded.\n");
        return;
    }

    fscanf(file, "%d %d\n", &player.health, &player.inventory_count);
    for (int i = 0; i < player.inventory_count; i++) {
        char buffer[MAX_DESC];
        fgets(buffer, MAX_DESC, file);
        buffer[strcspn(buffer, "\n")] = 0;
        player.inventory[i] = malloc(sizeof(Item));
        player.inventory[i]->name = strdup(buffer);
    }

    Room *current = rooms;
    while (current) {
        char room_name[MAX_DESC];
        char creature_status[MAX_DESC];
        fscanf(file, "%s %s\n", room_name, creature_status);

        if (strcmp(current->name, room_name) == 0 && current->creature) {
            if (strcmp(creature_status, "NONE") == 0) {
                free(current->creature->name);
                free(current->creature->description);
                free(current->creature);
                current->creature = NULL;
            } else {
                current->creature->health = atoi(creature_status);
            }
        }

        current = current->right ? current->right : current->down;
    }

    fclose(file);
    printf("Game loaded: %s\n", filepath);
}

void show_options() {
    printf("Commands:\n");
    printf("- move <direction>: Move between rooms (up, down, left, right).\n");
    printf("- look: Inspect the current room.\n");
    printf("- inventory: View your inventory.\n");
    printf("- pickup <item>: Pick up the specified item.\n");
    printf("- attack: Attack the creature in the room.\n");
    printf("- save <file>: Save the game.\n");
    printf("- load <file>: Load the game.\n");
    printf("- exit: Exit the game.\n");
}

void command_loop() {
    char command[MAX_DESC];
    printf("Welcome to the dungeon exploration game!\n");
    show_options();
    while (1) {
        printf("> ");
        fgets(command, MAX_DESC, stdin);
        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "move", 4) == 0) {
            move(command + 5);
        } else if (strcmp(command, "look") == 0) {
            look();
        } else if (strcmp(command, "inventory") == 0) {
            inventory();
        } else if (strncmp(command, "pickup", 6) == 0) {
            pickup(command + 7);
        } else if (strcmp(command, "attack") == 0) {
            attack();
        } else if (strncmp(command, "save", 4) == 0) {
            save_game(command + 5);
        } else if (strncmp(command, "load", 4) == 0) {
            load_game(command + 5);
        } else if (strcmp(command, "exit") == 0) {
            break;
        } else {
            printf("Invalid command.\n");
            show_options();
        }
    }
}

int main() {
    init_game();
    command_loop();
    cleanup_game();
    return 0;
}
