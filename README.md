ccgen
-----

Tool for generating random credit card numbers in Track 1 and Track 2 format.
This tool is useful for analyzing malwares that read/scans other processes'
memory for credit card numbers.

The following options can be specified:

    -1           : generate track1 data
    -2           : generate track2 data
    -s <seed>    : seed for the random number generator. Useful for when you
                   need to be able to generate the same set of data again
    -d <delay>   : delay in milliseconds (default: 100 milliseconds) before
                   generating new numbers


Random Names
------------

Names are taken from the US census data found here:
https://www.census.gov/topics/population/genealogy/data/1990_census/1990_census_namefiles.html

e.g.
```
# generate track 1 and 2 every 1 second, set seed to zero so rerunning
# produces the same set of data
ccgen -1 -2 -s 0 -d 1000
```

Additional Information
----------------------

http://sagan.gae.ucm.es/~padilla/extrawork/magexam1.html



Lloyd Macrohon <jl.macrohon@gmail.com>

