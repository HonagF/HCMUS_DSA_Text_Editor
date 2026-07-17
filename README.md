## KHÔNG code trực tiếp hay Push thẳng lên branch này

Nhánh `dev` dùng để ghép nối các module code từ các nhánh tính năng (`feat/...`) lại với nhau để chạy thử. Khi nào `dev` chạy được, không bug thì gộp vào nhánh `main`.

## Quy trình đưa code vào nhánh DEV

1. Mọi người chỉ viết code và Commit trên nhánh riêng của mình (ví dụ: `feat/trie`).
2. Khi code xong, lên trang web GitHub, tạo **Pull Request (PR)** yêu cầu gộp nhánh vào nhánh `dev`.
3. Nếu an toàn, Hoàng bấm **Merge** để gộp code vào `dev`.

## Hướng dẫn biên dịch 

Đã xong function linked list DLL cơ bản. Muốn test thì pull về rồi vào terminal trong VS code gõ

```bash
mingw32-make
.\Editor.exe
