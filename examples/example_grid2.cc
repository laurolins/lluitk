#include "lluitk/grid2.hh"

using namespace lluitk::grid2;

int main() {
    
    Grid2 grid;
    
    auto slot1 = grid.insert(nullptr);
    auto slot2 = grid.insert(nullptr);
    auto slot3 = grid.insert(nullptr);
    

    grid.window({0.0,0.0,300.0,100.0});
    grid.update();
    // auto slot2 = grid.insert(w2);
    // auto slot3 = grid.insert(w3);
    
    
    // reorganize later
    
    NodeIterator it(grid._root.get());
    Node* u;
    while ( (u = it.next()) ) {
        std::cout << (u->is_division() ? (u->as_division()->type() == HORIZONTAL ? "division(H) " : "division(V) ") : "slot: ") << u->window() << std::endl;
    }
    
    
}