
#include "sots2.h"	//For SOTS2 specific functions

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int _dowildcard = -1; //enable argument wildcard globbing in MingGW64


//Forward Declaration of our help function
void ShowHelp(void);

int main(int argc, char* argv[])
{

    //ios_base::sync_with_stdio(false); //Improve cout speed by disabling stdio synchronization

	std::ifstream iFile;				//File handler objects (input file, output file)
	std::ofstream oFile;
	
	std::string iName;					//File name strings
	std::string oName;
	
	std::vector<std::string> iNames;	//Vector to hold input file names
    
    int   iNamesCount = 0;				//Counter for number of names stored

    bool convertToSOTS = false;			//Whether to convert from DDS to SOTS format

	//If user specified more than just the executable path on the command line, parse arguments
    if (argc >= 2)
    {
		std::vector<std::string> arguments;
		for (int i = 1; i < argc; i++)
		{
			arguments.push_back(argv[i]);
		}
        
		//If the user specified "Help" argument at any point in the command, just show help and exit
        for(unsigned int i=1; i<arguments.size(); i++)
        {
			if(arguments.at(i) == "-H" || arguments.at(i) == "/H" || arguments.at(i) == "-h" || arguments.at(i) == "/h" || arguments.at(i) == "-?" || arguments.at(i) == "/?")
           {
                ShowHelp();
                exit (-1);
           }
        }

		//If no help requested, parse the arguments normally
        for(int i=0; i+1<=arguments.size(); i++)
        {
            //if it looks like a parameter switch, test it. 
            if(arguments.at(i).at(0) == '-' || arguments.at(i).at(0) == '/')	//Check for a switch prefix character (/ or -)
            {
                if(arguments.at(i).length() != 2)
                {
                    std::cout << "Error: Invalid Parameter " << arguments.at(i) << std::endl;
                    exit(-1);
                }
                else if(arguments.at(i).at(1) == 'o' or arguments.at(i).at(1) == 'O')
                {
                    if(i < (argc - 1))
                    {
                        if(arguments.at(i+1).length() > FILENAME_MAX)
                        {
                            std::cout << "Error: OUTPUT File Name exceeds " << FILENAME_MAX << "characters" << std::endl;
                            exit(-1);
                        }
                        else
                        {
							oName = arguments.at(i + 1);
                            i = argc; //break out of loop (stop looking at the rest of the parameters)
                        }
                    }
                    else
                    {
                        std::cout << "Error: Output parameter specified but no output file name provided" << std::endl;
                    }
                }
                else if(arguments.at(i).at(1) == 's' or arguments.at(i).at(1) == 'S')
                {
                    convertToSOTS = true;
                }
                else
                {
                    std::cout << "Error: Invalid Parameter " << arguments.at(i) << std::endl;
                }
            }
            else
            {
                //Not processing a parameter switch, so must be a file name, store it in our vector of file name strings
				iNames.push_back(arguments.at(i));
				++iNamesCount;
            }
        }

    }
    else ShowHelp();


    if(iNamesCount < 1)
    {
        std::cout << "Error: No INPUT File Name(s) provided" << std::endl;
    }
    else
    {
        for(int i=0; i<iNamesCount; i++)
        {
            std::cout << "Processing INPUT file: " << iNames.at(i) << std::endl;

			iFile.open(iNames.at(i), std::ifstream::in | std::ifstream::binary);
            if(iFile.fail())
            {
                std::cout << "Error: Unable to open INPUT filename " << iNames.at(i) << std::endl;
                exit (-1);
            }


            if(oName.length())
            {
                std::cout << "Processing OUTPUT File: " << oName << std::endl;
				oFile.open(oName, std::ofstream::out | std::ofstream::binary);
                if(oFile.fail())
                {
                    std::cout << "Error: Unable to open OUTPUT filename " << oName << std::endl;
                }


                std::cout << "===============================================================" << std::endl;

                if(convertToSOTS == true)
                {
                    ProcessDDSFile(iFile, oFile, oName);
                }
                else
                {
                    ProcessSots2File(iFile, oFile, oName);
                }
            }
            else
            {
                std::cout << "===============================================================" << std::endl;
                if(convertToSOTS == true)
                {
                    ProcessDDSFile(iFile, oFile, iNames.at(i));
                }
                else
                {
                    ProcessSots2File(iFile, oFile, iNames.at(i));
                }
            }

            iFile.close();

            std::cout << "===============================================================\n\n" << std::endl;
        }
    }

    return 1;
}

void ShowHelp(void)
{
    std::cout << std::endl;
    std::cout << "Sword of the Stars 2 Texture Extractor" << std::endl;
    std::cout << std::endl;
    std::cout << "This utility can convert SOTS2 .tga~ files into DirectDraw Surface (.DDS) files." << std::endl;
    std::cout << "DDS Output format type is automatically determined, based on the internal .tga~ file structure." << std::endl;
    std::cout << std::endl;
    std::cout << "Command Syntax: EXTRACT [-H|-?] [-S] InputFile [InputFile...] [-O OutputFile]" << std::endl;
    std::cout << std::endl;
    std::cout << "\t-H\\-?\t\t\tThis Help" << std::endl;
    std::cout << std::endl;
    std::cout << "\tInputFile\t\tInput File Name(s). (Sword of the Stars 2 .tga~ files)" << std::endl;
    std::cout << std::endl;
    std::cout << "\t-S\t\t\tConverts DirectDraw Surface (DDS) File to SOTS2 File (Optional)" << std::endl;
    std::cout << "\t\t\t\tIf enabled, InputFile(s) must be DDS files." << std::endl;
    std::cout << std::endl;
    std::cout << "\t-O OutputFile\t\tOutput File Name. (Optional)" << std::endl;
    std::cout << "\t\t\t\tProcessed file(s) will be saved to this file name." << std::endl;
    std::cout << std::endl;
    exit (1);
}
