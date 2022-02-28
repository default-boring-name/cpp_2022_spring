#include <iostream>
#include <algorithm>
#include <fstream>
#include <random>
#include <stdio.h>

std::random_device rdev;
std::mt19937 r_gen(rdev());
std::uniform_int_distribution<std::mt19937::result_type> d2(0, 1);
std::uniform_int_distribution<std::mt19937::result_type> d10(0, 9);

enum State {Dislocation, Atom};
enum Direction {Left, Right};
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
                return "■";
            }
            return " ";
        }
};
class Crystal{
    private:
        Cell* matrix;
        unsigned int size;
        bool running;
    public:
        Crystal(bool* scheme, unsigned int size){
            this->size = size;
            this->running = true;

            this->matrix = new Cell[size];
            for(int i = 0; i < size; i++){
                State cell_state = (scheme[i]) ? Dislocation : Atom;
                this->matrix[i].create(cell_state);
            }

            this->matrix[0].deactivate();
            this->matrix[this->size - 1].deactivate();
        }
        ~Crystal(){
            delete[] this->matrix;
        }
        void display(){
            for (int j = 0; j < 2 * this->size + 1; j++){
                std::cout << "--";
            }
            std::cout << "\n";
            for (int i = 0; i < this->size; i++){
                std::cout << " | " << this->matrix[i].to_str();
            }
            std::cout << " |\n";
            for (int j = 0; j < 2 * this->size + 1; j++){            
                std::cout << "--";
            }                                                     
            std::cout << "\n";
        }
        bool is_running(){
            return this->running;
        }
        void check_activity(){
            this->running = false;
            for (int i = 0; i < this->size; i++){

                if(this->matrix[i].is_active() && 
                   this->matrix[i].get_state() == Dislocation){
                    this->running = true;
                    i = this->size;
                }
            }
        }
        void update_activity(){
            for (int i = 1; i < this->size - 1; i++){
                if (this->matrix[i].get_state() == Dislocation){
                    Cell* left =  &this->matrix[i - 1];
                    Cell* right =  &this->matrix[i + 1];

                    if (left->get_state() == Dislocation
                        || right->get_state() == Dislocation){
                        this->matrix[i].deactivate();
                    }
                }
            }
        }
        void calculate_state(){
            for (int i = 1; i < this->size - 1; i++){
                if (this->matrix[i].is_active() 
                    && this->matrix[i].get_state() == Dislocation){

                    Direction dir = Direction(d2(r_gen));
                    Cell* target;
                    switch (dir){
                        case Left:
                            target = &this->matrix[i - 1];
                            break;
                        case Right:
                            target = &this->matrix[i + 1];
                            break;
                    }
                    if (target->get_future() == Atom){
                        target->set_future(Dislocation);
                    }
                    else{
                        this->matrix[i].set_future(Dislocation);
                    }
                }
            }
        }
        void update_state(){
            for (int i = 0; i < this->size; i++){
                this->matrix[i].update_state();
                if (this->matrix[i].is_active()){
                    this->matrix[i].set_future(Atom);
                }
            }
        }
};

int cycle(bool* scheme, int size){
    int iter = 0;
    Crystal crystal(scheme, size);
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
    unsigned int N = size;
    unsigned int K = disloc_number;

    bool* scheme = new bool[size];
    for(int i = 0; i < size; i++){
        scheme[i] = false;
    }

    for (int k = 0; k < repeat_number; k++){
        std::string bitmask(K, 1);
        bitmask.resize(N, 0);
        do {
            for (int i = 0; i < size; i++){
                scheme[i] = false;
            }
            for (int i = 0; i < N; ++i)
            {
                if (bitmask[i]){
                    scheme[i] = true; 
                } 
            }
            move_number += cycle(scheme, size);
            cycle_number += 1;
        } while (std::prev_permutation(bitmask.begin(), bitmask.end()));   
    }
    return (long double)(move_number) / cycle_number;
}
int main(){
     
    std::ofstream ratio_file("ratio_data", std::ios::out);
    int repeat_number = 100;
    for (int size = 6; size <= 20; size++){

        for (int disloc_number = 1; disloc_number <= size; disloc_number++){
            double ratio = disloc_number * 1.0 / size;
            ratio_file << ratio << " " 
                       << test_run(disloc_number, size, repeat_number) << "\n"; 
        }
    }

    ratio_file.close();


    return 0;
}

