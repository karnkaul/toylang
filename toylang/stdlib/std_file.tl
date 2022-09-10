fn file_read(path) {
	return _file("read", path);
}

fn file_write(file, path) {
	return _file("write", path, file.contents);
}
