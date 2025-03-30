# 🛡️ Battle City - SDL2 Remake

Một phiên bản đơn giản của trò chơi **Battle City** (game xe tăng) được viết bằng **C++** sử dụng thư viện **SDL2**. Trò chơi mang phong cách cổ điển, nơi người chơi điều khiển xe tăng để tiêu diệt kẻ địch và bảo vệ căn cứ.

---

## 🚀 Tính năng

- Giao diện menu chính (Start, Exit).
- Điều khiển xe tăng người chơi: di chuyển và bắn đạn theo 4 hướng.
- Xe tăng địch được sinh ra ngẫu nhiên, có AI đơn giản và có thể bắn.
- Hiệu ứng nổ khi đạn va chạm với xe tăng hoặc tường.
- Quản lý số mạng người chơi.
- Bản đồ được tải từ file `map.txt`.
- Hiển thị điểm số và kết thúc game khi người chơi thua.
- Được xây dựng hoàn toàn bằng SDL2, dễ mở rộng và phát triển thêm.

---

## 🖥️ Cấu trúc project

```bash
📁 battle_city/
├── Bullet.h / Bullet.cpp       # Xử lý đạn và va chạm
├── EnemyTank.h / EnemyTank.cpp # AI và xử lý xe tăng địch
├── Game.h / Game.cpp           # Logic tổng thể và quản lý game
├── Tank.h / Tank.cpp           # Xe tăng người chơi
├── Wall.h / Wall.cpp           # Vật cản và căn cứ
├── main.cpp                    # Hàm main khởi động game
├── tanks.png                   # Sprite sheet cho game
├── map.txt                     # File bản đồ
├── *.dll                       # Các thư viện cần thiết để chạy
└── README.md                   # Mô tả project
