## KHÔNG code trực tiếp hay Push thẳng lên branch này

Nhánh `dev` dùng để ghép nối các module code từ các nhánh tính năng (`feat/...`) lại với nhau để chạy thử. Khi nào `dev` chạy được, không bug thì gộp vào nhánh `main`.

## Quy trình đưa code vào nhánh DEV

1. Mọi người chỉ viết code và Commit trên nhánh riêng của mình (ví dụ: `feat/trie`).
2. Khi code xong, lên trang web GitHub, tạo **Pull Request (PR)** yêu cầu gộp nhánh vào nhánh `dev`.
3. Nếu an toàn, Hoàng bấm **Merge** để gộp code vào `dev`.

## Hướng dẫn biên dịch 

Khi các file `.c` và `.h` từ các nhánh đã được gom chung vào thư mục `src/`, mở Terminal tại thư mục gốc của đồ án và chạy lệnh sau để biên dịch toàn bộ chương trình:

```bash
gcc src/*.c -o dsa_notepad.exe
dsa_notepad.exe
