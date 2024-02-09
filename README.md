# Copyfile-basedon-extension
基于C++17制作的----以文件的扩展名来复制文件的工具
### 配置文件
```bash
config.json
{
    "disk": [ "C:\\"],        //遍历的地址,可以是文件夹路径
    "diskall" : 1,            //遍历所有盘符,开启为1,开启后disk将不会生效
    "fextension": [ ".zip", ".rar", ".7z" ],//复制文件的扩展名
    "expath": ["C:\\Windows"],//不遍历的目录
    "maximumf": 30,           //一个文件夹中最大文件夹/文件数量,如果大于则跳过这个目录
    "fsizelimit": 1000        //复制文件的大小限制
} 
```
## Contributors

[![Contributors](https://contributors-img.web.app/image?repo=Xpercent-YX/Xpercent-YX)](https://github.com/Xpercen/Copyfile-basedon-extension/graphs/contributors)
