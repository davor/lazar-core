/* Copyright (C) 2005  Christoph Helma <helma@in-silico.de>


    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <getopt.h>

#include "feature-generation.h"

using namespace std;

//! generate all linear fragments
int main(int argc, char *argv[]) {

    int status = 0;
    int c;
    bool t_file, a_file = false;
    char * structure_file = NULL;
    char * alphabet_file = NULL;

    typedef MolVect<OBLazMol,OBLinFrag,bool> OBLazMolVect ;

    while ((c = getopt(argc, argv, "s:a:")) != -1) {
        switch (c) {
        case 's':
            structure_file = optarg;
            t_file = true;
            break;
        case 'a':
            alphabet_file = optarg;
            a_file = true;
            break;
        case ':':
            status = 1;
            break;
        case '?':
            status = 1;
            break;
        }
    }

    shared_ptr<ConsoleOut> out;
    out.reset(new ConsoleOut());

    if (status | !t_file | !a_file) {
        *out << "usage: " << argv[0] << " -s id_and_smiles -a table_of_elements\n";
        out->print_err();
        return(status);
    }


    OBLazMolVect* structures = new OBLazMolVect(structure_file, out); // Create structures as normal pointer
                                                                      // Free-ed in ~FeatGen()!
    shared_ptr<FeatGen<OBLazMol,OBLinFrag,bool> > fragments ( new FeatGen<OBLazMol,OBLinFrag,bool>(alphabet_file,structures, out) );

    fragments->generate_linfrag();

    return (0);

}
