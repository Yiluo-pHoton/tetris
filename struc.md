classes:
Piece
	score
	shape
	next_shape
	x
	y
	width
	height
	next_width
	next_height
	box_shape
	next_box_shape
	box_map
	is_over

	initialize()
	set_shape()
	score_next()
	judge()
	rotate()
	switch_status()
	is_overlap()
	is_row_occupied()

Game
	score
	gameBoard

	newGame
	draw
