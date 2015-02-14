#!/usr/bin/python

##
# @file   makefileofsize.py
# @author Eric Turner <elturner@eecs.berkeley.edu>
# @brief  This program writes a file of a given size
#
# @section DESCRIPTION
#
# Will write specified file to disk.  The contents of the file will be
# garbage, but the file will be of a size specified to the program.
#

import argparse

##
# The main function will be called on execution of this code
#
def main():
    
    # set up argument parser object
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--size', required=False, \
            default=[1048576], nargs=1, type=int, \
            help=('The size of the output file, specified in bytes.'))
    parser.add_argument('outfile', type=str, \
            help=('Relative path to the file to export.'))

    # get arguments
    args    = parser.parse_args()
    outfile = args.outfile
    size    = args.size[0]

    # determine breakdown of file size
    kilobytes = size / 1024
    singles   = size % 1024

    # open the file
    fid = open(outfile, 'w')
    example_kilo = 'kilobyte'*128;
    example_byte = 'b'

    # write it
    for i in xrange(kilobytes):
        fid.write(example_kilo)
    for i in xrange(singles):
        fid.write(example_byte)

    # clean up
    fid.close()

##
# Boilerplate code to call main function
#
if __name__ == '__main__':
    main()
