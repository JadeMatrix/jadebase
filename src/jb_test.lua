-- dofile( "" )

name, major, minor, patch = jade.utility.get_program_version()

test_window = jade.windowsys.new_window();
test_window:set_title( name .. " " .. major .. "." .. minor .. "." .. patch .. " (Lua)" )

-- Callbacks

function quit()
    jade.utility.quit()
end
quit_cb = jade.utility.new_callback( quit )

-- Group A

group_a = jade.gui.new_group( 0, 0, 256, 256 )

b = jade.gui.new_button( 10, 10, 150, 27 )
t = jade.gui.new_text_rsrc( 12, jade.gui.LABEL_FONT, "Hello World" );
t:baseline( false )
b:set_contents( t, jade.gui.CENTER_CENTER )

b:set_toggle_on_callback( quit_cb )

group_a:add_element( b )

-- Group B

group_b = jade.gui.new_group( 0, 0, 256, 256 )

group_b:add_element( jade.gui.new_dial( 10, 10 ) )
group_b:add_element( jade.gui.new_dial( 62, 10, true ) )

-- Group C

group_c = jade.gui.new_group( 0, 0, 256, 256 )

-- Tabset

tab_a = jade.gui.new_tab( "Roman Text testtesttesttest", group_a )
tab_b = jade.gui.new_tab( "日本語のテキスト", group_b )
-- tab_b = jade.gui.new_tab( "متن فارسی", group_b )
tab_c = jade.gui.new_tab( "טקסט בעברית", group_c )

test_tabset = jade.gui.new_tabset( 0, 0, 256, 256 )

test_tabset:add_tab( tab_a )
test_tabset:add_tab( tab_b )
test_tabset:add_tab( tab_c )

-- Add to window

test_window:top_group():add_element( test_tabset )

-- test_window:top_group():add_element( b )