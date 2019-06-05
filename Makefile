all : project3

project3 : main.cpp
        g++ main.cpp -o project3
clean:
        rm -rf program*
