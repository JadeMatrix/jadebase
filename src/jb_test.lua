-- dofile( "" )

name, major, minor, patch = jade.utility.get_program_version()

jade.utility.log( name, " ", major, ".", minor, ".", patch, "\n" )
jade.utility.log( "hello world ", 123, "\n" )

jade.utility.log( "jb_BlockExponent: ", jade.utility.get_setting( "jb_BlockExponent" ), "\n" )

jade.utility.quit()