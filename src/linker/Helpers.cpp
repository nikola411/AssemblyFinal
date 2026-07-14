#include "Helpers.hpp"

#include "Linker.hpp"

#include <fstream>

int ParseArguments(std::shared_ptr<Linker> linker, int argc, char *argv[])
{
    int i = 0;
    States currentState = READ_NEXT;
    std::vector<std::string> args(argv + 1, argv + argc);

    while (currentState != States::FINISH)
    {
        switch (currentState)
        {
            case READ_NEXT:
            {
                if (i >= (int)args.size())
                {
                    currentState = States::ERROR;
                    break;
                }

                if (args[i] == "-o")
                {
                    currentState = States::SET_OUTPUT;
                    i++;
                    continue;
                }

                if (StartsWith(args[i], "-place"))
                {
                    currentState = States::SET_PLACE;
                    continue;
                }

                if (args[i] == "-hex")
                {
                    currentState = States::SET_HEX;
                    continue;
                }

                if (args[i] == "-relocatable")
                {
                    currentState = States::SET_RELOC;
                    continue;
                }

                if (EndsWith(args[i], ".o"))
                {
                    currentState = States::SET_INPUT;
                    continue;
                }

                currentState = States::ERROR;
                break;

                //error
            }
            case SET_HEX:
            {
                linker->SetMode(Modes::HEX);
                currentState = States::READ_NEXT;
                i++;
                break;
            }
            case SET_RELOC:
            {
                linker->SetMode(Modes::RELOC);
                currentState = States::READ_NEXT;
                i++;
                break;
            }
            case SET_PLACE:
            {
                std::vector<std::string> parts = Split(args[i], '=');
                std::vector<std::string> place = Split(parts[1], '@');
                linker->SetPlace(place[0], place[1]);
                currentState = States::READ_NEXT;
                i++;
                break;
            }
            case SET_OUTPUT:
            {
                linker->SetOutput(args[i]);
                currentState = States::READ_NEXT;
                i++;
                break;
            }
            case SET_INPUT:
            {
                if (i == args.size())
                {
                    currentState = States::FINISH;
                    break;
                }
                linker->SetInput(args[i]);
                currentState = States::SET_INPUT;
                i++;
                break;
            }
            default:
                return 1;
        }
    }

    return 0;
}

/* citamo binarni fajl koji ce da ima sledeci format:
hhhhhhhh hh hh hh hh hh hh hh hh  hh hh hh hh hh hh hh hh
gde je h neka heksadecimalna cifra koja predstavlja 4bita
*/
std::vector<uint8_t> ReadBinaryFile(const std::string &path)
{
    std::vector<uint8_t> output = {};
    std::ifstream file(path);
    if (!file.is_open())
        return output;

    std::string line;
    while (std::getline(file, line))
    {
        auto parts = Split(line, ' ');
        for (int i = 1; i < (int)parts.size(); i++)
        {
            if (parts[i].empty())
                continue;
            output.push_back(std::stoi(parts[i], nullptr, 16));
        }
    }

    return output;
}
