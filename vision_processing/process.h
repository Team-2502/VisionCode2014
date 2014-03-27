#ifndef PROCESS_H
#define PROCESS_H

#include <vector>
#include "../types.h"

std::vector <Target> processAndGetTargets(int width, int height, unsigned char * data);
void showClientImage();

#endif // PROCESS_H
