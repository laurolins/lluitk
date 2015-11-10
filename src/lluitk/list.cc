#include "list.hh"

#include <fstream>


namespace lluitk {
    
    namespace list {
        
        static std::uint64_t RENDER_LOOP_ITERATON = 0;
        
        static std::ofstream& wheelos() {
            static std::unique_ptr<std::ofstream> ptr;
            if (!ptr) {
                ptr.reset(new std::ofstream("/Users/llins/Desktop/wheel.psv"));
                auto &os = *ptr.get();
                os << "time|delta|iter" << std::endl;
            }
            return *ptr.get();
        }
        

        //----------------
        // SpeedupWheel
        //----------------
        
        SpeedupWheel::SpeedupWheel() {
            _starttime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        }
        
        double SpeedupWheel::speedup(llsg::Vec2 move) {
            auto current_ts = std::chrono::high_resolution_clock::now().time_since_epoch().count();;
            
            wheelos() << (current_ts - _starttime) << "|" << move.y() << "|" << RENDER_LOOP_ITERATON << std::endl;
            
            if (move != _move) {
                _factor  = 1.0;
                _repeats = 1;
                _move    = move;
                _ts      = current_ts;
            }
            else {
                auto dt = current_ts - _ts;
                if (dt < 1000000000ULL) { // 25ms
                    if ((_repeats % 1) == 0) {
                        //if (_factor < 128) {
                        _factor += 1;
                        std::cout << "speed up " << _factor << " reps: " << _repeats << std::endl;
                        //}
                    }
                    _ts = current_ts;
                    ++_repeats;
                }
                else {
                    std::cout << "cut move" << std::endl;
                    _factor  = 1.0;
                    _repeats = 1;
                    _move    = move;
                    _ts      = current_ts;
                }
            }
            return _factor;
        }
        
    }
    
}