#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

typedef struct
{
    char name[256];
    char phone[11];
    int year, paid, vacced;
} applicant;

#define HARCRA_FEL SIGUSR1

void flush()
{ //flushing newline characters from input stream that get stuck after scanf() calls and disrupt fgets() calls
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

applicant *load_data(applicant *applicants, int *array_size, int *taken_size)
{
    FILE *database = fopen("data.txt", "r");
    if (database != NULL)
    {
        char temp[274];
        while (fgets(temp, 274, database) != NULL)
        {
            char name[256] = "";
            char *parts = strtok(temp, " ");

            //getting the name from the tokenized string (could have any number of parts, e. g. Spanish names)
            //reading stops when it encounters a digit (signaling that the parser is at the age of the applicant)
            while (!isdigit(parts[0]))
            {
                strcat(name, parts);
                parts = strtok(NULL, " ");
                if (!isdigit(parts[0]))
                    strcat(name, " ");
            }
            strcpy(applicants[*array_size].name, name);

            //getting the remaining data about the applicant
            applicants[*array_size].year = atoi(parts);
            parts = strtok(NULL, " ");
            strcpy(applicants[*array_size].phone, parts);
            parts = strtok(NULL, " ");
            applicants[*array_size].paid = atoi(parts);
            parts = strtok(NULL, " ");
            applicants[*array_size].vacced = atoi(parts);

            *array_size += 1;
            if (*array_size == *taken_size)
            {
                *taken_size *= 2;
                applicants = realloc(applicants, *taken_size * sizeof(applicant));
            }
            strcpy(name, "");
        }
        fclose(database);
        return applicants;
    }
    else
    {
        perror("A fajl megnyitasa sikertelen volt!");
        return NULL;
    }
}

void save_data(applicant applicants[], int array_size)
{
    FILE *database;
    database = fopen("data.txt", "w");
    int error = database == NULL ? 1 : 0;
    while (error)
    {
        perror("A fajl megnyitasa sikertelen volt!");
        char response;
        int correct_response = 1;
        while (correct_response)
        {
            printf("Ujraprobalod a mentest? (I/N)");
            scanf("%c", &response);
            correct_response = (tolower(response) == 'i' || tolower(response) == 'n') ? 0 : 1;
        }
        if (response == 'n')
            return;
        else
        {
            database = fopen("data.txt", "w");
            error = database == NULL ? 1 : 0;
        }
    }

    for (int i = 0; i < array_size; i++)
    {
        fprintf(database, "%s %d %s %d %d\n", applicants[i].name, applicants[i].year, applicants[i].phone, applicants[i].paid, applicants[i].vacced);
    }
    fclose(database);
}

int menu()
{
    int choice;
    printf("\nA megfelelo gomb lenyomasaval valaszd ki a kivant menupontot!\n");
    printf("-------------------------------------------------------------\n");
    printf("1 - Uj jelentkezo rogzitese\n");
    printf("2 - Jelentkezo adatainak modositasa\n");
    printf("3 - Jelentkezo torlese\n");
    printf("4 - Jelentkezok listazasa\n");
    printf("5 - Oltobuszok inditasa\n");
    printf("0 - Kilepes\n");
    printf("Menupont: ");
    scanf("%d", &choice);

    return choice;
}

void print_all_applicants(applicant *applicants, int array_size)
{
    printf("\nAz osszes jelentkezo, listazva: \n");
    printf("----------------------------------\n");
    for (int i = 0; i < array_size; i++)
    {
        printf("%d) %s %d %s %s %s \n", i + 1, applicants[i].name, applicants[i].year, applicants[i].phone, applicants[i].paid ? "fizetett" : "nem fizetett", applicants[i].vacced ? "OLTVA" : "");
    }
}

void get_applicant_data(applicant applicants[], int index, int needs_flush)
{
    if (needs_flush)
        flush();
    printf("Jelentkezo neve: ");
    fgets(applicants[index].name, 256, stdin);
    size_t name_length = strlen(applicants[index].name);
    applicants[index].name[name_length - 1] = '\0'; //deleting the newline character stuck from fgets

    printf("Jelentkezo szuletesi eve: ");
    scanf("%d", &applicants[index].year);
    flush();
    printf("Jelentkezo telefonszama: ");
    fgets(applicants[index].phone, 12, stdin);

    flush();
    char buff;
    printf("Fizetett-e a jelentkezo: (I/N)");
    scanf("%c", &buff);
    if (tolower(buff) == 'i')
        applicants[index].paid = 1;
    else if (tolower(buff) == 'n')
        applicants[index].paid = 0;
    applicants[index].vacced = 0;
}

void new_applicant(applicant applicants[], int *array_size, int *taken_size)
{
    printf("\nUj jelentkezo rogzitese:\n");
    printf("------------------------\n");

    get_applicant_data(applicants, *array_size, 1);
    *array_size = *array_size + 1;
    if (*array_size == *taken_size)
    {
        *taken_size *= 2;
        applicants = realloc(applicants, *taken_size * sizeof(applicant));
    }
    printf("Sikeres jelentkezes!\n");
}

void modify_applicant(applicant applicants[], int array_size)
{
    printf("Kinek az adatait akarod megvaltoztatni? (nev)");
    int found = 0, index = 0;
    char name[256];
    flush();

    fgets(name, 256, stdin);
    size_t name_length = strlen(name);
    name[--name_length] = '\0';

    for (int i = 0; i < array_size; i++)
    {
        if (strcmp(applicants[i].name, name) == 0)
        {
            found = 1;
            index = i;
        }
    }
    if (found)
        get_applicant_data(applicants, index, 0);
    else
    {
        printf("Nem talaltam ilyen nevu embert! Index alapjan valaszd ki, melyik jelentkezot szeretned modositani.");
        print_all_applicants(applicants, array_size);
        printf("Index: ");
        scanf("%d", &index);
        get_applicant_data(applicants, --index, 1);
    }
    printf("Sikeres modositas!\n");
}

void delete_applicant(applicant applicants[], int *array_size)
{
    printf("Melyik jelentkezot toroljem?");
    char to_delete[256];
    flush();
    fgets(to_delete, 256, stdin);
    size_t name_length = strlen(to_delete);
    to_delete[name_length - 1] = '\0';

    int count = 0;
    int indices[*array_size];
    for (int i = 0; i < *array_size; i++)
    {
        if (strcmp(applicants[i].name, to_delete) == 0)
        {
            indices[count] = i;
            ++count;
        }
    }

    if (count < 1)
    {
        printf("Nincs ilyen nevu jelentkezo a rendszerben. Kerlek add meg a torlendo jelentkezo sorszamat!");
        print_all_applicants(applicants, *array_size);
        printf("Sorszam: ");
        scanf("%d", &indices[0]);
        indices[0] -= 1;
    }
    else if (count > 1)
    {
        printf("Tobb ilyen nevu jelentkezo rogzitve van a rendszerben. Kerlek add meg a torlendo jelentkezo sorszamat!\n");
        for (int i = 0; i < count; i++)
        {
            printf("%d) %s %d %s %d \n", indices[i] + 1, applicants[indices[i]].name, applicants[indices[i]].year, applicants[indices[i]].phone, applicants[indices[i]].paid);
        }
        printf("Sorszam: ");
        scanf("%d", &indices[0]);
        indices[0] -= 1;
    }

    if (indices[0] == *array_size - 1)
        *array_size = *array_size - 1;
    else
    {
        for (int i = indices[0]; i < *array_size - 1; i++)
            applicants[i] = applicants[i + 1];
        *array_size = *array_size - 1;
    }
    printf("Sikeres torles!\n");
}

void signal_handler(int signumber)
{
    printf("\nHarcra fel!\n");
}

int get_applicants_to_vaccinate(applicant applicants[], int start, int indices[], int bus_number)
{
    //this function collects and prints the data of the applicants that will be sent of for vaccination to one of the buses
    //returns the index of the last applicant that will be vaccinated, so the next process can start from there
    //the indices of the found people will be stored in the indices[] array, so the sending algorithm doesn't have to search again
    printf("\n%d) oltobuszra varjak: \n", bus_number);
    printf("---------------\n");
    int found = 0;
    int i = start;
    while (found < 5)
    {
        if (applicants[i].vacced == 0)
        {
            indices[found] = i;
            printf("%d) %s %d %s %s\n", i + 1, applicants[i].name, applicants[i].year, applicants[i].phone, applicants[i].paid ? "fizetett" : "nem fizetett");
            found++;
        }
        i++;
    }
    printf("\n");
    return i;
}

void bus(applicant applicants[], int pipe1[], int pipe2[], int bus_number)
{
    close(pipe1[1]);
    close(pipe2[0]);
    sleep(bus_number);
    kill(getppid(), HARCRA_FEL);

    int registered_at_bus[5];
    int succesfully_vaccinated[5]; //indices of applicants that successfully got vaccinated
    int number = 0;

    //reading the indices of the applicants from the pipe, sending back the indices of who's got vacced
    for (int i = 0; i < 5; i++)
    {
        read(pipe1[0], &registered_at_bus[i], sizeof(int));
        number = rand() % 10;
        if (number > 1)
            succesfully_vaccinated[i] = registered_at_bus[i];
        else
            succesfully_vaccinated[i] = -1;

        write(pipe2[1], &succesfully_vaccinated[i], sizeof(int));
    }

    printf("%d) oltobuszra erkezett adatok: \n", bus_number);
    printf("--------------------- \n");
    for (int i = 0; i < 5; i++)
    {
        int ind = registered_at_bus[i];
        printf("%d) %s %d %s %s\n", i + 1, applicants[ind].name, applicants[ind].year, applicants[ind].phone, applicants[ind].paid ? "fizetett" : "nem fizetett");
    }
}

void directorate_of_operations(applicant applicants[], int pipe1[], int pipe2[], int *startpoint, int bus)
{ //sending applicant data to buses, registering successful vaccinations
    int indices_to_send[5];
    int vaccinated_indices[5];
    *startpoint = get_applicants_to_vaccinate(applicants, *startpoint, indices_to_send, bus);

    //sending the indices of the applicants and reading the succesfully vaccinated indices
    //also setting those people as 'vacced'
    for (int i = 0; i < 5; i++)
    {
        write(pipe1[1], &indices_to_send[i], sizeof(int));
        read(pipe2[0], &vaccinated_indices[i], sizeof(int));
        if (vaccinated_indices[i] != -1)
            applicants[vaccinated_indices[i]].vacced = 1;
    }
}

void new_day(applicant applicants[], int array_size)
{
    srand(time(0));
    int need_vaccines = 0;
    for (int i = 0; i < array_size; i++)
    {
        if (applicants[i].vacced == 0)
            need_vaccines++;
    }

    if (need_vaccines < 5)
        printf("Nincs eleg jelentkezo oltobusz inditasahoz!\n");

    if (need_vaccines > 4)
    {
        int status;
        int startpoint = 0; //the index where the search for unvaccinated applicants starts from
        int *startpointer = &startpoint;

        int pipe1[2]; //pipe for sending applicant data to buses (child processes)
        pipe(pipe1);

        int pipe2[2]; //pipe for sending back who got vaccinated successfully
        pipe(pipe2);

        pid_t child_process = fork();
        if (child_process < 0)
        { //start of the first new process failed
            perror("Az elso oltobusz inditasa sikertelen volt!");
            return;
        }

        if (child_process > 0)
        {
            if (need_vaccines > 9)
            {
                pid_t child_process2 = fork();

                if (child_process2 < 0)
                { //start of the second new process failed
                    perror("A masodik oltobusz inditasa sikertelen volt!");
                    return;
                }

                if (child_process2 > 0)
                { //parent process with two children

                    close(pipe1[0]);
                    close(pipe2[1]);

                    pause();
                    directorate_of_operations(applicants, pipe1, pipe2, startpointer, 1);

                    pause();
                    directorate_of_operations(applicants, pipe1, pipe2, startpointer, 2);

                    waitpid(child_process, &status, 0);
                    waitpid(child_process2, &status, 0);
                }
                else
                { //child process 2
                    bus(applicants, pipe1, pipe2, 2);
                    exit(0);
                }
            }
            else
            { //parent process with one child
                close(pipe1[0]);
                close(pipe2[1]);
                pause();
                directorate_of_operations(applicants, pipe1, pipe2, startpointer, 2);

                waitpid(child_process, &status, 0);
            }
        }
        else
        { //child process 1
            bus(applicants, pipe1, pipe2, 1);
            exit(0);
        }
    }
}

int main()
{
    signal(HARCRA_FEL, signal_handler);
    int running = 1;
    applicant *applicants = (applicant *)malloc(sizeof(applicant) * 5);
    int size = 0;
    int allocated_size = 5;
    int *size_ptr = &size; //the version of C on the server doesn't support passing by reference, so we need a pointer for modifying its value
    int *allocated_ptr = &allocated_size;
    applicants = load_data(applicants, size_ptr, allocated_ptr);

    do
    {
        int option = menu();
        if (option)
        {
            switch (option)
            {
            case 1:
                new_applicant(applicants, size_ptr, allocated_ptr);
                break;
            case 2:
                modify_applicant(applicants, size);
                break;
            case 3:
                delete_applicant(applicants, size_ptr);
                break;
            case 4:
                print_all_applicants(applicants, size);
                break;
            case 5:
                new_day(applicants, size);
                break;
            default:
                printf("Ervenytelen bemenet!\n");
                break;
            }
        }
        else
            running = 0;
    } while (running);

    save_data(applicants, size);
    free(applicants);
    return 0;
}

//modositas/torles: nev alapjan -> lehet tobb van, listazas (tomb meretenek csokkentese? koltsegesebb masolgatni mint hagyni egy csomó helyet?)
//ha nincs olyan nev, osszes ember listazasa, sorszam alapjan torles / modositas
//tipushibak/merethibak kezelese összes bevitelkor (nev nem lehet hosszabb 256 karakternél) (legyen típusonként külön pl readInt?)
//sorszambekereskor ellenorzes, nem kisebb mint 0 vagy nagyott, mint a tomb merete!
//stringek trimmelése névbeolvasáskor a végéről?