-- dofile( "" )

jade.gui.new_group( 0, 0, 256, 256 )

function gen_tabset()
    -- Group A
    
    group_a = jade.gui.new_group( 0, 0, 256, 256 )
    
    b = jade.gui.new_button( 10, 10, 150, 27 )
    t = jade.gui.new_text_rsrc( 12, jade.gui.LABEL_FONT, "Hello World" );
    t:baseline( false )
    b:set_contents( t, jade.gui.CENTER_CENTER )
    
    b:set_toggle_on_callback( jade.utility.new_callback( function () jade.windowsys.new_window() end ) )
    b:set_toggle_off_callback( jade.utility.new_callback( function () collectgarbage() end ) )
    
    group_a:add_element( b )
    
    -- Group B
    
    group_b = jade.gui.new_group( 0, 0, 256, 256 )
    
    group_b:add_element( jade.gui.new_dial( 10, 10 ) )
    group_b:add_element( jade.gui.new_dial( 62, 10, true ) )
    
    -- Tabset
    
    test_tabset = jade.gui.new_tabset( 0, 0, 256, 256 )
    
    test_tabset:add_tab( jade.gui.new_tab( "Roman Text testtesttesttest", group_a ) )
    test_tabset:add_tab( jade.gui.new_tab( "متن فارسی", group_b ) )
    test_tabset:add_tab( jade.gui.new_tab( "日本語のテキスト", jade.gui.new_group( 0, 0, 256, 256 ) ) )
    test_tabset:add_tab( jade.gui.new_tab( "טקסט בעברית", jade.gui.new_group( 0, 0, 256, 256 ) ) )
    
    return test_tabset
end

-- for i=1,2345 do
--     jade.utility.log( "hello" )
-- end

name, major, minor, patch = jade.utility.get_program_version()

test_window = jade.windowsys.new_window()
test_window:set_title( name .. " " .. major .. "." .. minor .. "." .. patch .. " (Lua)" )

-- Add to window

test_window:add_element( gen_tabset() )