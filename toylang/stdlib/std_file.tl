struct File {
	var contents;
	var last_modified;
}

fn file_open(path) {
	var ret = File();
	ret.contents = _file("read", path);
	ret.last_modified = _file("last_modified", path);
	return ret;
}


fn file_write(file, path) {
	return _file("write", path, file.contents);
}
