# Đồ án CTDL & GT: Mini Text Editor

Đây là kho lưu trữ mã nguồn cho đồ án môn Cấu trúc dữ liệu và Giải thuật. Dự án mô phỏng một Text Editor chạy trên Console bằng ngôn ngữ C, hỗ trợ tính năng Undo/Redo và Auto-complete.

Toàn bộ mã nguồn `.c` và `.h` được gom chung vào thư mục `src/`. Không tạo thêm folder mới để tránh lỗi đường dẫn khi biên dịch.
- `src/`: Nơi chứa toàn bộ code C (`main.c`, `list.c`, `stack.c`, `trie.c`).
- `data/`: Chứa file `dictionary.txt` phục vụ cây Trie.
- `docs/`: Chứa slide thuyết trình và file Word báo cáo.

---

## Không làm theo = gay

Để tránh lỗi xung đột code, mỗi người code một branch. Đã tạo sẵn nhánh cho từng người. Làm đúng theo 4 bước sau mỗi khi code:

### Bước 1: Cập nhật nhánh từ mạng (Trước khi code)
1. Mở GitHub Desktop.
2. Bấm nút **Fetch origin** để tải các cập nhật mới nhất từ mạng về.
3. Bấm vào nút **Current Branch**, tìm tên nhánh bạn được phân công (ví dụ: `feat/undo-redo`) và click chọn.
4. Xác nhận lại nút Current Branch đã hiện đúng tên nhánh của mình. *(Không code khi đang ở nhánh `main` hoặc `dev`)*.

### Bước 2: Viết Code
1. Mở thư mục project.
2. Vào thư mục `src/`, tạo hoặc mở file `.c` / `.h` và bắt đầu code.
3. **Lưu ý:** Không chạm vào các file `.gitkeep`, không sửa code trong file của người khác, muốn sửa gì thì review code. 

### Bước 3: Commit
1. Quay lại GitHub Desktop, danh sách các file mình vừa sửa/tạo mới hiện ở cột bên trái.
2. Nhập **Summary**.
   * *Ví dụ: "Hoàn thành hàm Push cho Undo Stack"*
3. Bấm **Commit to [tên-nhánh-của-bạn]**.

### Bước 4: Đẩy code lên mạng (Push)
- Sau khi Commit xong, bấm nút **Push origin** (ở thanh menu trên cùng hoặc ngay giữa màn hình) để đưa code lên GitHub.
- Xong hết thì request để merge branch vào dev.
