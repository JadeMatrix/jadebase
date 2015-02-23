-- dofile( "" )

name, major, minor, patch = jade.utility.get_program_version()

jade.utility.log( name, " ", major, ".", minor, ".", patch, "\n" )
jade.utility.log( "hello world ", 123, "\n" )

jade.utility.log( "jb_BlockExponent: ", jade.utility.get_setting( "jb_BlockExponent" ), "\n" )

jade.utility.set_setting( "test_setting", "stringval" )

testfile = jade.filetypes.png.new( "resources/gui_resources.png" )
w, h = testfile:dimensions()
ct = testfile:color_type()
if ct == jade.filetypes.png.GRAY then ct = "GRAY"
    elseif ct == jade.filetypes.png.PALETTE then ct = "PALETTE"
        elseif ct == jade.filetypes.png.RGB then ct = "RGB"
            elseif ct == jade.filetypes.png.RGB_ALPHA then ct = "RGB_ALPHA"
                elseif ct == jade.filetypes.png.GRAY_ALPHA then ct = "GRAY_ALPHA"
                    else ct = "????"
                    end
jade.utility.log( "Opened ", testfile, ":\n",
                  "    - dimensions: ", w, "x", h, "\n",
                  "    - bit depth: ", testfile:bit_depth(), "\n",
                  "    - color type: ", ct, "\n" )

jade.utility.log( "exiting now\n" )

jade.utility.quit()