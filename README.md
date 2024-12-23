# WHU-OSLab
WHU OS Project

武汉大学网安院操作系统结课实验

## 编译

```bash
gzip -cd 80m.img.gz > 80m.img
gzip -cd a.img.gz > a.img
make image
```

## 运行&调试

更改`bochsrc`文件的`romimage`，使用支持gdb的bochs调试

```bash
bochs -q -f bochsrc
```

启动使用口令在check_passwd函数中检验，具体值在commit记录中查找

## TODO

- 文件加密功能与edit不兼容，原因是文件加密未考虑调整文件偏移
- 改进随机数算法
- 改进签名算法
