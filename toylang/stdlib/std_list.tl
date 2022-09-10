struct List {
	var value;
	var prev;
	var next;
}

fn list_make(value) {
	var ret = List();
	ret.value = value;
	return ret;
}

fn list_head(node) {
	while (node.prev != null) {
		node = node.prev;
	}
	return node;
}

fn list_tail(node) {
	while (node.next != null) {
		node = node.next;
	}
	return node;
}

fn list_size(head) {
	var ret = 0;
	while (head != null) {
		ret = ret + 1;
		head = head.next;
	}
	return ret;
}

fn list_push_back(head, value) {
	while (head.next != null) {
		head = head.next;
	}
	head.next = List();
	head.next.value = value;
	head.next.prev = head;
	return head.next;
}

fn list_pop_back(head) {
	while (head.next != null) {
		head = head.next;
	}
	if (head.prev != null) {
		head.prev.next = null;
	}
	head.prev = null;
	return head;
}

fn list_for_each(head, func) {
	while (head != null) {
		func(head.value);
		head = head.next;
	}
}
