extends Node2D

const WIDTH = 800
const HEIGHT = 1000

var id = 0
var symbol = "while"
var y = 0
var x = 0
var parent_id = -1
var parent_node = null


# Called when the node enters the scene tree for the first time.
func _ready():
	self.position.y = self.y * HEIGHT
	self.position.x = self.x * WIDTH
	for i in get_parent().get_children():
		if i.id == self.parent_id:
			self.parent_node = i
			break
	self.visible = false;
	self.find_node("Label").text = symbol
	self.add_to_group("nodes")
	if(symbol.length() >= 3):
		$Label.rect_scale = Vector2(0.6, 0.7)
	
	lineup()
	pass # Replace with function body.

func lineup():
	if parent_node:
		$Line2D.set_point_position(1,parent_node.position - self.position)
		self.visible = true

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
