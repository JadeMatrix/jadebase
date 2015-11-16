-- dofile( "" )

jade.gui.new_group( 0, 0, 256, 256 )

window_counter = 1
function window_name()
    name = jade.utility.get_setting( "jb_test_WindowName" ) .. " #" .. window_counter
    window_counter = window_counter + 1
    return name
end

function gen_tabset()
    -- Group A (Buttons)
    
    group_a = jade.gui.new_group( 0, 0, 256, 256 )
    
    b = jade.gui.new_button( 10, 10, 150, 27 )
    t = jade.gui.new_text_rsrc( 12, jade.gui.LABEL_FONT, "Hello World" );
    t:baseline( false )
    b:set_contents( t, jade.gui.CENTER_CENTER )
    
    b:set_toggle_on_callback( jade.utility.new_callback(
        function ()
            new_test_window = jade.windowsys.new_window()
            new_test_window:set_title( window_name() )
        end
    ) )
    b:set_toggle_off_callback( jade.utility.new_callback( function () collectgarbage() end ) )
    
    group_a:add_element( b )
    
    -- Group B (Dials)
    
    group_b = jade.gui.new_group( 0, 0, 256, 256 )
    
    group_b:add_element( jade.gui.new_dial( 10, 10 ) )
    group_b:add_element( jade.gui.new_dial( 62, 10, true ) )
    
    -- Group C (Scrollsets)
    
    inner_group = jade.gui.new_group( 0, 0, 256, 256 )
    -- inner_group:add_element( jade.gui.new_button( 10, 10, 30, 300 ) )
    -- inner_group:add_element( jade.gui.new_button( 40, 10, 300, 30 ) )
    inner_group:add_element( jade.gui.new_button( 10, 10, 30, 700 ) )
    inner_group:add_element( jade.gui.new_button( 40, 10, 700, 30 ) )
    inner_group:add_element( jade.gui.new_button( 250, 250, 50, 50 ) )
    
    test_scrollset = jade.gui.new_scrollset( 0, 0, 220, 200, inner_group )
    -- test_scrollset:bars_always_visible( true )
    
    group_c = jade.gui.new_group( 0, 0, 256, 256 )
    group_c:add_element( test_scrollset )
    
    -- Group D (empty for now)
    
    group_d = jade.gui.new_group( 0, 0, 256, 256 )
    
    -- Tabset
    
    test_tabset = jade.gui.new_tabset( 0, 0, 256, 256 )
    
    test_tabset:add_tab( jade.gui.new_tab( "Roman Text testtesttesttest", group_a ) )
    test_tabset:add_tab( jade.gui.new_tab( "متن فارسی", group_b ) )
    test_tabset:add_tab( jade.gui.new_tab( "日本語のテキスト", group_c ) )
    test_tabset:add_tab( jade.gui.new_tab( "טקסט בעברית", group_d ) )
    
    return test_tabset
end

test_window = jade.windowsys.new_window()

-- name, major, minor, patch = jade.utility.get_program_version()
-- test_window:set_title( name .. " " .. major .. "." .. minor .. "." .. patch .. " (Lua)" )

test_window:set_title( window_name() )

-- Add to window

test_window:add_element( gen_tabset() )