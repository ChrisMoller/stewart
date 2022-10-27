/***
    This file is part of the Stewart platform simulator.

    Copyright 2022 C. H. L. Moller

Stewart is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

Stewart is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
Stewart. If not, see <https://www.gnu.org/licenses/>.
 ***/

#ifndef POPEN2
#define POPEN2
FILE * popen2(const char *command, const char *type, int * pid);
int pclose2(FILE * fp, pid_t pid);
#endif // POPEN2


