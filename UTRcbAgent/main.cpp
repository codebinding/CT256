#include "rotorcontrol.h"

#include <QCoreApplication>
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <unistd.h>

#include <QDir>

const std::vector<std::string> split(const std::string& s, const char& c){

    std::string buff{""};
    std::vector<std::string> v;

    for(auto n:s){

        if(n != c){
            buff += n;
        }
        else if(buff != ""){

            v.push_back(buff);
            buff = "";
        }
    }

    if(buff != ""){

        v.push_back(buff);
    }

    return v;
}

const std::vector<uint8_t> split(const std::string& hex_string, const std::string& delimitors){

    std::string my_substr{""};
    std::vector<uint8_t> my_int_vector;

    for(auto c:hex_string){

        if(delimitors.find(c) == std::string::npos){

            my_substr += c;
        }
        else if(my_substr != ""){

            my_int_vector.push_back(std::stoi(my_substr, 0, 16));
            my_substr = "";
        }
    }

    if(my_substr != ""){

        my_int_vector.push_back(std::stoi(my_substr, 0, 16));
    }

    return my_int_vector;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::ifstream my_inf;

    my_inf.open((QDir::homePath() + "/Development/Input.txt").toStdString());

    if(!my_inf){

        std::cout << "Cannot open input file" << std::endl;
        return EXIT_FAILURE;
    }

    RotorControl *rc = new RotorControl();
    rc->Initialize();

    std::string my_line;
    std::vector<uint8_t> my_elements;

    CANFrame8 *my_frame = new CANFrame8();

    std::getline(my_inf, my_line);
    while (!my_inf.eof()) {

        my_elements = split(my_line, ", ");
        uint16_t my_id = my_elements[0];
        uint8_t *my_data = &my_elements[1];

        my_frame->SetFrame(my_id, my_data);
        std::cout <<  my_frame->ToString() <<std::endl;

        for(int i = 0; i < 1000L; i++){

            rc->SendFrame(*my_frame);
        }

        std::getline(my_inf, my_line);
    }

    return a.exec();
}
