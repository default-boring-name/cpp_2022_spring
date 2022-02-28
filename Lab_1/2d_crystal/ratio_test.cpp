#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <random>
#include <stdio.h>

std::random_device rdev;
std::mt19937 r_gen(rdev());
std::uniform_int_distribution<std::mt19937::result_type> d4(0, 3);
std::uniform_int_distribution<std::mt19937::result_type> d10(0, 9);

enum State {Dislocation, Atom};
enum Direction {Left, Down, Up, Right};
class Cell{
    private:
        bool active;
        State state;
        State future;
    public:
        Cell(){
            this->active = false;
            this->future = Atom;
            this->state = Atom;
        };
        void create(State state){
            this->active = true;
            this->future = Atom;
            this->state = state;
        }
        void deactivate(){
            this->active=false;
            this->future = this->state;
        }
        void set_future(State future){
            this->future = future;
        }
        void update_state(){
            this->state = this->future;
        }
        bool is_active(){
            return this->active;
        }
        State get_state(){
            return this->state;
        }
        State get_future(){
            return this->future;
        }
        std::string to_str(){
            if (this->state == Dislocation){
                return "8";
            }
            return " ";
        }
};

class Crystal{
    private:
        Cell** matrix;
        unsigned int height;
        unsigned int width;
        bool running;
    public:
        Crystal(int** scheme, unsigned int height, unsigned int width){
            this->height = height;
            this->width = width;
            this->running = true;

            this->matrix = new Cell*[height];
            for(int i = 0; i < height; i++){
                this->matrix[i] = new Cell[width];
                for(int j = 0; j < width; j++){
                    State cell_state = (scheme[i][j] == 1) ? Dislocation : Atom;
                    this->matrix[i][j].create(cell_state);
                }
            }

            for (int i = 0; i < this->height; i++){
                this->matrix[i][0].deactivate();
                this->matrix[i][this->width - 1].deactivate();
            }
            for (int j = 0; j < this->width; j++){
                this->matrix[0][j].deactivate();
                this->matrix[this->height - 1][j].deactivate();
            }
        }
        ~Crystal(){
            for(int i = 0; i < height; i++){
                delete[] this->matrix[i];
            }
            delete[] this->matrix;
        }
        void display(){
            for (int j = 0; j < 2 * this->width + 1; j++){
                std::cout << "--";
            }
            std::cout << "\n";
            for (int i = 0; i < this->height; i++){
                for (int j = 0; j < this->width; j++){
                    std::cout << " | " << this->matrix[i][j].to_str();
                }
                std::cout << " |\n";
                for (int j = 0; j < 2 * this->width + 1; j++){
                    std::cout << "--";
                }
                std::cout << "\n";
            }
        }
        bool is_running(){
            return this->running;
        }
        void check_activity(){
            this->running = false;
            for (int i = 0; i < this->height; i++){
                for (int j = 0; j < this->width; j++){

                    if(this->matrix[i][j].is_active() && 
                       this->matrix[i][j].get_state() == Dislocation){

                        this->running = true;
                        i = this->height;
                        j = this->width;
                    }
                }
            }
        }
        void update_activity(){
            for (int i = 1; i < this->height - 1; i++){
                for (int j = 1; j < this->width - 1; j++){
                    if (this->matrix[i][j].get_state() == Dislocation){
                        Cell* top =  &this->matrix[i - 1][j];
                        Cell* bottom =  &this->matrix[i + 1][j];
                        Cell* left =  &this->matrix[i][j - 1];
                        Cell* right =  &this->matrix[i][j + 1];

                        if (top->get_state() == Dislocation 
                            || bottom->get_state() == Dislocation
                            || left->get_state() == Dislocation
                            || right->get_state() == Dislocation){
                           
                            this->matrix[i][j].deactivate();
                        }
                    }
                }
            }
        }
        void calculate_state(){
            for (int i = 1; i < this->height - 1; i++){
                for (int j = 1; j < this->width - 1; j++){
                    if (this->matrix[i][j].is_active() 
                        && this->matrix[i][j].get_state() == Dislocation){

                        Direction dir = Direction(d4(r_gen));
                        Cell* target;
                        switch (dir){
                            case Left:
                                target = &this->matrix[i][j - 1];
                                break;
                            case Down:
                                target = &this->matrix[i + 1][j];
                                break;
                            case Up:
                                target = &this->matrix[i - 1][j];
                                break;
                            case Right:
                                target = &this->matrix[i][j + 1];
                                break;
                        }
                        if (target->get_future() == Atom){
                            target->set_future(Dislocation);
                        }
                        else{
                            this->matrix[i][j].set_future(Dislocation);
                        }
                    }
                }
            }
        }
        void update_state(){
            for (int i = 0; i < this->height; i++){
                for (int j = 0; j < this->width; j++){
                    this->matrix[i][j].update_state();
                    if (this->matrix[i][j].is_active()){
                        this->matrix[i][j].set_future(Atom);
                    }
                }
            }
        }
};

int cycle(int** scheme, int size){
    int iter = 0;
    Crystal crystal(scheme, size, size);
    while (crystal.is_running()){

        crystal.update_activity();
        crystal.check_activity();
        crystal.calculate_state();
        crystal.update_state();
        iter++;
        if (iter > 1000000){
            break;
        }
    }
    return iter - 1;
}
long double test_run(unsigned int disloc_number, unsigned int size, int repeat_number){
    long long unsigned int move_number = 0;
    long long unsigned int cycle_number = 0;
    unsigned int N = size * size;
    unsigned int K = disloc_number;

    int** scheme = new int*[size];
    for(int i = 0; i < size; i++){
        scheme[i] = new int[size];
        for(int j = 0; j < size; j++){
            scheme[i][j] = 0;
        }
    }

    for (int k = 0; k < repeat_number; k++){
        std::string bitmask(K, 1);
        bitmask.resize(N, 0);
        do {
            for(int i = 0; i < size; i++){
                for(int j = 0; j < size; j++){
                    scheme[i][j] = 0;
                }
            }
            for (int i = 0; i < N; ++i)
            {
                if (bitmask[i]){
                    scheme[i / size][i % size] = 1; 
                } 
            }
            move_number += cycle(scheme, size);
            cycle_number +=1;
        } while (std::prev_permutation(bitmask.begin(), bitmask.end()));   
    }
    return (long double)(move_number) / cycle_number;
}
int main(){
     
    std::ofstream ratio_file("ratio_data", std::ios::out);
    int repeat_number = 1000;
    for (int size = 1; size <= 5; size++){
        std::cout << "size = " << size << "\n";
        if (size == 4){
            repeat_number = 100;
        }
        if (size == 5){
            repeat_number = 10;
        }
        for (int disloc_number = 1; disloc_number <= size * size; disloc_number++){
            double ratio = disloc_number * 1.0 / (size * size);
            std::cout << disloc_number << "\n";
            ratio_file << ratio << " " 
                       << test_run(disloc_number, size, repeat_number) << "\n"; 
        }
        std::cout << std::endl;
    }

    ratio_file.close();
    return 0;
}

