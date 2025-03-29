for (test in 0...10) {
	System.print(test.toString)
}

var seq_ = 0..<10
var iter_ = null
while (iter_ = seq_.iterate(iter_)) {
	var test = seq_.iteratorValue(iter_)
	System.print(test.toString)
}

