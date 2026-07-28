import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# ========== 1. 坐标与数据准备 ==========
x = np.linspace(0, 200, 200)  # X轴范围 0-200
y = np.linspace(0, 120, 120)  # Y轴范围 0-120
X, Y = np.meshgrid(x, y)

# 模拟带圆环的分布（替换为你的真实Z二维矩阵即可）
center_x, center_y = 100, 60
radius = np.sqrt((X - center_x)**2 + (Y - center_y)**2)
Z = 5000 * np.exp(-radius**2 / (2 * 80**2)) + 1000 * np.exp(-((radius - 40)**2) / (2 * 10**2))
Z = np.clip(Z, 0, 5000)

# ========== 2. 创建画布与3D坐标轴 ==========
fig = plt.figure(figsize=(11, 7), dpi=100, facecolor='white')
ax = fig.add_subplot(111, projection='3d')
ax.set_facecolor('white')  # 纯白背景，匹配示例图

# ========== 3. 绘制实体块效果（顶面+底座+四周侧面） ==========
# 3.1 顶面彩色曲面（核心数据面）
surf = ax.plot_surface(
    X, Y, Z,
    cmap='plasma',       # 蓝-紫-橙-黄渐变，完全匹配示例配色
    edgecolor='none',    # 关闭网格线
    alpha=1.0,
    shade=True,          # 开启光影，增强立体感
    antialiased=True     # 抗锯齿，边缘更平滑
)

# 3.2 底部深色底座（Z=0平面）
Z_bottom = np.zeros_like(Z)
ax.plot_surface(
    X, Y, Z_bottom,
    color='#1a0040',     # 深紫蓝色，对应色条最底端
    edgecolor='none',
    alpha=1.0
)

# 3.3 四周垂直侧面（形成厚度感）
# 左侧面 x=0
Y_left, Z_scale = np.meshgrid(y, np.linspace(0, 1, 2))
Z_left = Z_scale * Z[:, 0][np.newaxis, :]
X_left = np.full_like(Z_left, 0)
ax.plot_surface(X_left, Y_left, Z_left, color='#2d0066', edgecolor='none')

# 右侧面 x=200
Y_right, Z_scale = np.meshgrid(y, np.linspace(0, 1, 2))
Z_right = Z_scale * Z[:, -1][np.newaxis, :]
X_right = np.full_like(Z_right, 200)
ax.plot_surface(X_right, Y_right, Z_right, color='#2d0066', edgecolor='none')

# 前侧面 y=0
X_front, Z_scale = np.meshgrid(x, np.linspace(0, 1, 2))
Z_front = Z_scale * Z[0, :][np.newaxis, :]
Y_front = np.full_like(Z_front, 0)
ax.plot_surface(X_front, Y_front, Z_front, color='#2d0066', edgecolor='none')

# 后侧面 y=120
X_back, Z_scale = np.meshgrid(x, np.linspace(0, 1, 2))
Z_back = Z_scale * Z[-1, :][np.newaxis, :]
Y_back = np.full_like(Z_back, 120)
ax.plot_surface(X_back, Y_back, Z_back, color='#2d0066', edgecolor='none')

# ========== 4. 坐标轴样式美化 ==========
ax.set_xlabel('X', fontsize=11, labelpad=10)
ax.set_ylabel('Y', fontsize=11, labelpad=10)
ax.set_zlabel('Z', fontsize=11, labelpad=10)

ax.set_xlim(0, 200)
ax.set_ylim(0, 120)
ax.set_zlim(0, 5500)

# 浅灰虚线网格 + 关闭轴面板填充，更简洁
ax.xaxis.pane.fill = False
ax.yaxis.pane.fill = False
ax.zaxis.pane.fill = False
ax.grid(color='lightgray', linestyle='--', linewidth=0.5)

# ========== 5. 色条与视角 ==========
# 右侧独立色条，匹配示例图样式
cbar = fig.colorbar(surf, shrink=0.75, pad=0.12)
cbar.set_label('数值大小', fontsize=10)
cbar.ax.tick_params(labelsize=9)

# 视角微调，高度还原示例图的倾斜角度
ax.view_init(elev=22, azim=-58)

# 隐藏多余边框
for spine in ax.spines.values():
    spine.set_visible(False)

plt.tight_layout()
plt.show()

