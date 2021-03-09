extends Control


var NODE_SCENE = load("res://node.tscn")

func load_file(file):
	var reg = RegEx.new()
	reg.compile("id=(\\d+)\\ssymbol=(.+)\\sy=(.+)\\sx=(.+)\\sparent_id=(\\d+)")
	var f = File.new()
	var err = f.open(file, File.READ)
	print(err)
	var index = 1
	while not f.eof_reached(): # iterate through all lines until the end of file is reached
		var line = f.get_line()
		if(line.length() == 0):
			break;
		var result = reg.search(line)
		var node = NODE_SCENE.instance()
		node.id = int(result.get_string(1))
		node.symbol = result.get_string(2)
		node.y = float(result.get_string(3))
		node.x = float(result.get_string(4))
		node.parent_id = int(result.get_string(5))
		$node_root.add_child(node)
		yield(get_tree().create_timer(.05), "timeout")
		print(node.symbol)
	f.close()
	return

func write_file(file):
	var f = File.new()
	f.open(file, File.WRITE)
	f.store_string($TextEdit.text)
	f.close()
	
func load_log(file):
	var f = File.new()
	f.open(file, File.READ)
	var text = f.get_as_text()
	$TextEdit3.text  = text


func _ready():
	pass

func draw():
	pass

func _on_Button_pressed():
	for i in $node_root.get_children():
		i.queue_free()
	write_file("input.txt")
	var list = []
	OS.execute("syntaxer.exe",list)
	load_file("output.txt")
	yield(get_tree().create_timer(.1), "timeout")
	load_log("log.txt")
	pass # Replace with function body.
