To run:
    g++ $(pkg-config --cflags --libs opencv) -std=c++11  shapefinder.cpp -o shapefinder

    ./shapefinder -i triangle -c 0x33aaff -o triangle_output -s triangle