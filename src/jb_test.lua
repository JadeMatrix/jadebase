-- dofile( "" )

name, major, minor, patch = jade.utility.get_program_version()

test_window = jade.windowsys.new_window();
test_window:set_title( name .. " " .. major .. "." .. minor .. "." .. patch )

-- Replicates GUI test code that used to be in jb_window.cpp -------------------

-- Group A

test_group_a = jade.gui.new_group( test_window, 0, 0, 256, 256 )

b = jade.gui.new_button( test_window, 10, 10, 150, 27 )
t = jade.gui.new_text_rsrc( 12, jade.gui.GUI_LABEL_FONT, "Hello World" );
t:enable_baseline( false )
b:set_contents( t, jade.gui.CENTER_CENTER )
test_group_a:add_element( b )

--[[
-- Group B

test_group_b = jade.gui.new_group( test_window, 0, 0, 256, 256 )

test_group_b:add_element( jade.gui.new_dial( test_window, 10, 10 ) )
test_group_b:add_element( jade.gui.new_dial( test_window, 62, 10, true ) )

-- Group C

test_group_c = jade.gui.new_group( test_window, 0, 0, 256, 256 )

-- Tabset

test_tabset = jade.gui.new_tabset( test_window, 0, 0, 256, 256 )
]]

test_tabset:add_tab( test_group_a, "Roman Text testtesttesttest" )
--[[
test_tabset:add_tab( test_group_b, "日本語のテキスト" )
-- test_tabset:add_tab( test_group_b, "متن فارسی" )
test_tabset:add_tab( test_group_c, "טקסט בעברית" )
]]

-- Add to window

test_window:get_top_group():add_element( test_tabset, 0, 0 )

test_window:request_redraw()