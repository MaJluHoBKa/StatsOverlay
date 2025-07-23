import datetime
import os
import sys
from PyQt5.QtWidgets import (QApplication, QWidget, QLabel, QVBoxLayout, QHBoxLayout, 
                            QSizePolicy, QPushButton, QStackedWidget, QGridLayout, 
                            QScrollArea, QLineEdit, QMessageBox, QLayout, QMenu, QAction)
from PyQt5.QtCore import QPropertyAnimation, QEasingCurve, QRect, Qt, QSize, QTimer, QUrl, pyqtSignal
from PyQt5.QtGui import QPixmap, QIcon, QFontDatabase

from PyQt5.QtGui import QDesktopServices
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure
from server import APIClient
from datetime import datetime, timedelta
import threading
import time
from HWIDActivator import HWIDActivator

def resource_path(relative_path):
        import sys, os
        if hasattr(sys, '_MEIPASS'):
            return os.path.join(sys._MEIPASS, relative_path)
        return os.path.join(os.path.abspath("."), relative_path)

class Overlay(QWidget):
    def __init__(self, with_icon=False, offset_x=0, overlay_info=None):
        super().__init__()
        self.overlay_info = overlay_info
        self.overlay_info_visible = True

        self.setWindowFlags(Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint | Qt.Tool)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.resize(40, 40)

        screen_geometry = QApplication.primaryScreen().availableGeometry()
        self.move(0 + offset_x, (screen_geometry.height() - 40) // 2)

        font_path = resource_path('src/fonts/JetBrainsMono-Bold.ttf')  # Укажите путь к вашему шрифту
        font_id = QFontDatabase.addApplicationFont(font_path)
        if font_id != -1:
            font_family = QFontDatabase.applicationFontFamilies(font_id)[0]
        else:
            font_family = "Consolas"

        self.setStyleSheet("""
            background-color: rgba(30, 30, 30, 153);  /* Темный фон с прозрачностью */
            border: 1px solid #808080;  /* Темный бордер */
            border-radius: 10px;
        """)

        layout = QVBoxLayout()

        # Если нужно добавить иконку
        if with_icon:
            icon_label = QLabel(self)
            icon = QPixmap(resource_path('src/stats_icon.webp'))
            icon_label.setPixmap(icon)
            icon_label.setAlignment(Qt.AlignCenter)
            layout.addWidget(icon_label)

        self.setLayout(layout)
        self.setMouseTracking(True)
        self._drag_active = False

    def mousePressEvent(self, event):
            if event.button() == Qt.RightButton:
                self._drag_active = True
                self._drag_start_pos = event.globalPos()

            # Полный выход из приложения при правом клике
            if event.button() == Qt.LeftButton:
                if self.overlay_info:
                    if self.overlay_info_visible:
                        self.overlay_info.animate_hide()
                    else:
                        self.overlay_info.animate_show()
                    self.overlay_info_visible = not self.overlay_info_visible

    def mouseMoveEvent(self, event):
        if self._drag_active:
            delta = event.globalPos() - self._drag_start_pos
            new_x = self.x() + delta.x()
            new_y = self.y() + delta.y()

            self.move(new_x, new_y)
            self._drag_start_pos = event.globalPos()

    def mouseReleaseEvent(self, event):
        # Когда отпустили кнопку мыши, завершили перетаскивание
        if event.button() == Qt.RightButton:
            self._drag_active = False
            self.snap_to_closest_screen_edge()

    def snap_to_closest_screen_edge(self):
        screen_geometry = QApplication.primaryScreen().availableGeometry()
        margin = 10  # Радиус, на который окно будет привязываться к краю экрана

        # Получаем текущую позицию
        new_x = self.x()
        new_y = self.y()

        # Расстояние до каждого края экрана
        distance_left = abs(new_x - screen_geometry.left())
        distance_right = abs(new_x + self.width() - screen_geometry.right())
        distance_top = abs(new_y - screen_geometry.top())
        distance_bottom = abs(new_y + self.height() - screen_geometry.bottom())

        # Определяем, к какому краю привязать
        if distance_left < distance_right and distance_left < distance_top and distance_left < distance_bottom:
            new_x = screen_geometry.left()
        elif distance_right < distance_left and distance_right < distance_top and distance_right < distance_bottom:
            new_x = screen_geometry.right() - self.width()
        elif distance_top < distance_left and distance_top < distance_right and distance_top < distance_bottom:
            new_y = screen_geometry.top()
        elif distance_bottom < distance_left and distance_bottom < distance_right and distance_bottom < distance_top:
            new_y = screen_geometry.bottom() - self.height()

        self.move(new_x, new_y)

class Overlay_info(QWidget):
    def __init__(self, api_client, offset_x=0):
        super().__init__()
        self.api_client = api_client

        self.api_client.load_stats_from_file()
        if self.api_client.is_auth:
            self.api_client.set_unknow_tanks()
            self.api_client.prolong_token()

        self.is_auth = self.api_client.is_auth
        self.animation_running = False
        self.full_height = 200
        self.setWindowFlags(Qt.WindowStaysOnTopHint | Qt.FramelessWindowHint)
        self.setAttribute(Qt.WA_TranslucentBackground)
        self.resize(250, 200)  # Начальный размер

        # Привязка к центру экрана
        screen_geometry = QApplication.primaryScreen().availableGeometry()
        self.move(0 + offset_x, (screen_geometry.height() - self.height()) // 2)

        self.setStyleSheet("""
            background-color: rgba(30, 30, 30, 220);
            border: 0px solid #404040;
            border-radius: 0px;
        """)

        # Добавляем кнопку выхода -----------------------------------------------------------------------
        config_container = QWidget()
        config_container.setStyleSheet("""
            border-top-left-radius: 10px;
            border-bottom-left-radius: 10px;
        """)
        v_config_layout = QVBoxLayout()
        v_config_layout.setSpacing(0)
        v_config_layout.setContentsMargins(0, 0, 0, 0)

        self.stacked_widget = QStackedWidget()
        self.stacked_widget.setStyleSheet("""
            QStackedWidget, QStackedWidget > QWidget {
                background-color: rgba(30, 30, 30, 220);
                border: none;
                border-top-right-radius: 10px;
                border-bottom-right-radius: 10px;
            }
        """)
        self.stacked_widget.setContentsMargins(0, 0, 0, 0)
        self.stacked_widget.layout().setContentsMargins(0, 0, 0, 0)
        self.stacked_widget.layout().setSpacing(1)

        self.stream_page = Stream(api_client=self.api_client, config_container=config_container, stacked_widget=self.stacked_widget)
        self.stats_page = Stats(api_client=self.api_client, stream_page = self.stream_page)
        self.rating_page = Rating(api_client=self.api_client, stream_page = self.stream_page)
        self.tanks_page = TanksStat(api_client=self.api_client)
        self.graphics_page = Graphics(api_client=self.api_client)
        self.other_page = Other(api_client=api_client, stream_page = self.stream_page)
        self.info_page = Info(api_client=self.api_client, main_stat=self.stats_page, rating_stat = self.rating_page ,tank_stat=self.tanks_page, other_stat=self.other_page, stream_page = self.stream_page)

        self.stacked_widget.addWidget(self.info_page)
        self.stacked_widget.setCurrentIndex(0)

        self.stacked_widget.addWidget(self.stream_page)
        self.stacked_widget.setCurrentIndex(1)

        self.stacked_widget.addWidget(self.other_page)
        self.stacked_widget.setCurrentIndex(2)

        self.stacked_widget.addWidget(self.graphics_page)
        self.stacked_widget.setCurrentIndex(3)

        self.stacked_widget.addWidget(self.tanks_page)
        self.stacked_widget.setCurrentIndex(4)

        self.stacked_widget.addWidget(self.rating_page)
        self.stacked_widget.setCurrentIndex(5)

        self.stacked_widget.addWidget(self.stats_page)
        self.stacked_widget.setCurrentIndex(6)

        main_layout = QHBoxLayout()
        main_layout.setSpacing(0)
        main_layout.setContentsMargins(0, 2, 0, 2)

        self.stats_button = QPushButton(self)
        self.stats_button.setIcon(QIcon(QPixmap(resource_path('src/stats_icon.webp'))))
        self.stats_button.setIconSize(QSize(24, 24))
        self.stats_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 10px;
                border-bottom-left-radius: 0px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.stats_button.clicked.connect(lambda: self.switch_page(6))

        self.rating_button = QPushButton(self)
        self.rating_button.setIcon(QIcon(QPixmap(resource_path('src/rating_icon.webp'))))
        self.rating_button.setIconSize(QSize(24, 24))
        self.rating_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 0px;
                border-bottom-left-radius: 0px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.rating_button.clicked.connect(lambda: self.switch_page(5))

        self.tank_button = QPushButton(self)
        self.tank_button.setIcon(QIcon(QPixmap(resource_path('src/tanks_icon.webp'))))
        self.tank_button.setIconSize(QSize(24, 24))
        self.tank_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 0px;
                border-bottom-left-radius: 0px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.tank_button.clicked.connect(lambda: self.switch_page(4))

        self.graphics_button = QPushButton(self)
        self.graphics_button.setIcon(QIcon(QPixmap(resource_path('src/graphics_icon.png'))))
        self.graphics_button.setIconSize(QSize(24, 24))
        self.graphics_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 0px;
                border-bottom-left-radius: 0px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.graphics_button.clicked.connect(lambda: self.switch_page(3))

        self.other_button = QPushButton(self)
        self.other_button.setIcon(QIcon(QPixmap(resource_path('src/other_icon.png'))))
        self.other_button.setIconSize(QSize(24, 24))
        self.other_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 0px;
                border-bottom-left-radius: 0px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.other_button.clicked.connect(lambda: self.switch_page(2))

        self.stream_button = QPushButton(self)
        self.stream_button.setIcon(QIcon(QPixmap(resource_path('src/stream_icon.png'))))
        self.stream_button.setIconSize(QSize(24, 24))
        self.stream_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 0px;
                border-bottom-left-radius: 0px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.stream_button.clicked.connect(lambda: self.switch_page(1))

        self.info_button = QPushButton(self)
        self.info_button.setIcon(QIcon(QPixmap(resource_path('src/info_icon.webp'))))
        self.info_button.setIconSize(QSize(24, 24))
        self.info_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 0px;
                border-bottom-left-radius: 0px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.info_button.clicked.connect(lambda: self.switch_page(0))

        self.exit_button = QPushButton(self)
        self.exit_button.setIcon(QIcon(QPixmap(resource_path('src/exit_icon.webp'))))
        self.exit_button.setIconSize(QSize(24, 24))
        self.exit_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(30, 30, 30, 0);
                color: #e2ded3;
                font-size: 12px;
                font-family: Consolas;
                border-top-left-radius: 0px;
                border-bottom-left-radius: 10px;
            }
            QPushButton:hover {
                background-color: rgba(50, 50, 50, 100); /* Цвет при наведении */
            }
            QPushButton:pressed {
                background-color: rgba(70, 70, 70, 150); /* Цвет при нажатии */
            }
        """)
        self.exit_button.clicked.connect(self.quit)

        v_config_layout.addWidget(self.stats_button)
        v_config_layout.addWidget(self.rating_button)
        v_config_layout.addWidget(self.tank_button)
        v_config_layout.addWidget(self.graphics_button)
        v_config_layout.addWidget(self.other_button)
        v_config_layout.addWidget(self.stream_button)
        v_config_layout.addWidget(self.info_button)
        v_config_layout.addWidget(self.exit_button)
        config_container.setLayout(v_config_layout)
        main_layout.addWidget(config_container)


        self.stats_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        self.rating_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        self.tank_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        self.graphics_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        self.exit_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        self.info_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        self.other_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)
        self.stream_button.setSizePolicy(QSizePolicy.Preferred, QSizePolicy.Expanding)

        # -------------------------------------------------------------------------------------------------

        main_layout.addWidget(self.stacked_widget)
        self.setLayout(main_layout)
        self.switch_page(6)

    def switch_page(self, index):
        self.stacked_widget.setCurrentIndex(index)
        self.full_height = self.stacked_widget.currentWidget().sizeHint().height()
        # # Устанавливаем прозрачность фона QStackedWidget в зависимости от индекса
        # if index == 1:  # Если текущая вкладка — Stream
        #     self.stacked_widget.setStyleSheet("""
        #         QStackedWidget, QStackedWidget > QWidget {
        #             background-color: rgba(30, 30, 30, 0);  /* Полупрозрачный фон */
        #             border: none;
        #             border-top-right-radius: 10px;
        #             border-bottom-right-radius: 10px;
        #         }
        #     """)
        # else:
        #     self.stacked_widget.setStyleSheet("""
        #         QStackedWidget, QStackedWidget > QWidget {
        #             background-color: rgba(30, 30, 30, 220);  /* Стандартный фон */
        #             border: none;
        #             border-top-right-radius: 10px;
        #             border-bottom-right-radius: 10px;
        #         }
        #     """)

        # Изменяем размер окна только по ширине
        if self.isVisible():
            self.resize(self.width(), self.full_height)

    def mousePressEvent(self, event):
        if event.button() == Qt.LeftButton:
            self.drag_position = event.globalPos() - self.frameGeometry().topLeft()
            event.accept()
    
    def mouseMoveEvent(self, event):
        if event.buttons() == Qt.LeftButton:
            self.move(event.globalPos() - self.drag_position)
            event.accept()
    
    def mouseReleaseEvent(self, event):
        self.setCursor(Qt.ArrowCursor)
    
    def quit(self):
        if self.api_client.is_auth:
            self.api_client.save_stats_to_file()
        QApplication.quit()

    def animate_hide(self):
        if self.animation_running:
            return
        self.animation_running = True

        self.anim = QPropertyAnimation(self, b"geometry")
        self.anim.setDuration(200)
        self.anim.setEasingCurve(QEasingCurve.InOutCubic)

        current_geometry = self.geometry()
        target_geometry = QRect(current_geometry.x(), current_geometry.y(), current_geometry.width(), 0)

        self.anim.setStartValue(current_geometry)
        self.anim.setEndValue(target_geometry)

        self.opacity_anim = QPropertyAnimation(self, b"windowOpacity")
        self.opacity_anim.setDuration(200)
        self.opacity_anim.setStartValue(1)
        self.opacity_anim.setEndValue(0)

        self.anim.finished.connect(lambda: self.setVisible(False))
        self.anim.finished.connect(self._clear_animation_flag)

        self.anim.start()
        self.opacity_anim.start()

    def animate_show(self):
        if self.animation_running:
            return
        self.setVisible(True)
        self.animation_running = True

        self.anim = QPropertyAnimation(self, b"geometry")
        self.anim.setDuration(200)
        self.anim.setEasingCurve(QEasingCurve.InOutCubic)

        target_geometry = QRect(self.x(), self.y(), self.width(), self.full_height)
        start_geometry = QRect(self.x(), self.y(), self.width(), 0)

        self.setGeometry(start_geometry)

        self.anim.setStartValue(start_geometry)
        self.anim.setEndValue(target_geometry)

        self.opacity_anim = QPropertyAnimation(self, b"windowOpacity")
        self.opacity_anim.setDuration(200)
        self.opacity_anim.setStartValue(0)
        self.opacity_anim.setEndValue(1)

        self.anim.finished.connect(self._clear_animation_flag)

        self.anim.start()
        self.opacity_anim.start()

    def _clear_animation_flag(self):
        self.animation_running = False

class Stats(QWidget):   
    def __init__(self, api_client, stream_page):
        super().__init__()
        self.api_client = api_client
        self.stream_page = stream_page
        self.setup_ui()

        self.updating_thread = threading.Thread(target=self.update_stats_periodically)
        self.updating_thread.daemon = True
        self.updating_thread.start()
    
    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(0)

        self.labels = {}
        self.add_icon_with_label(layout, resource_path('src/gold_icon.webp'), "Золото", is_top=True)
        self.add_icon_with_label(layout, resource_path('src/silver_icon.webp'), "Кредиты")
        self.add_icon_with_label(layout, resource_path('src/xp_battle_icon.webp'), "Боевой опыт")
        self.add_icon_with_label(layout, resource_path('src/free_xp_icon.webp'), "Cвободный опыт")

        container = QWidget()
        container.setStyleSheet("background-color: transparent;")
        h_layout = QHBoxLayout()
        h_layout.setContentsMargins(10, 2, 10, 2)
        h_layout.setSpacing(10)

        iconArrow = QLabel()
        iconArrow.setPixmap(QPixmap(resource_path('src/arrow_icon.webp')))
        iconArrow.setStyleSheet("background-color: rgba(30, 30, 30, 0);")

        avgLabel = QLabel(self)
        avgLabel.setText("Боевая эффективность (AVG)")
        avgLabel.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 14px;
            letter-spacing: 0px;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        avgLabel.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        avgLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        h_layout.addWidget(iconArrow)
        h_layout.addWidget(avgLabel)

        container.setLayout(h_layout)
        layout.addWidget(container)

        self.add_icon_with_label(layout, resource_path('src/battles_icon.webp'), "Проведено боев")
        self.add_icon_with_label(layout, resource_path('src/win_icon.webp'), "Победы")
        self.add_icon_with_label(layout, resource_path('src/damage_icon.webp'), "Урон")
        self.add_icon_with_label(layout, resource_path('src/xp_icon.webp'), "Опыт", is_bottom=True)

        self.full_height = self.sizeHint().height()

        # Пример: обновление значений
        self.set_value("Золото", "-")
        self.set_value("Кредиты", "-")
        self.set_value("Боевой опыт", "-")
        self.set_value("Cвободный опыт", "-")
        self.set_value("Проведено боев", "-")
        self.set_value("Победы", "-")
        self.set_value("Урон", "-")
        self.set_value("Опыт", "-")

    def add_icon_with_label(self, layout, icon_path, key, is_top=False, is_bottom=False):
        container = QWidget()
        h_layout = QHBoxLayout()
        h_layout.setContentsMargins(10, 2, 10, 2)
        h_layout.setSpacing(10)

        icon = QLabel()
        icon.setPixmap(QPixmap(icon_path))
        icon.setStyleSheet("background-color: rgba(30, 30, 30, 0);")

        label = QLabel()

        label.setStyleSheet("""
            font-family: Consolas;
            font-size: 12px;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        label.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)

        self.labels[key] = label

        h_layout.addWidget(icon)
        h_layout.addWidget(label)

        container.setLayout(h_layout)

        radius_style = ""
        if is_top:
            radius_style += "border-top-right-radius: 10px;"
        if is_bottom:
            radius_style += "border-bottom-right-radius: 10px;"

        container.setStyleSheet(f"""
            background-color: rgba(30, 30, 30, 0);
            {radius_style}
        """)

        layout.addWidget(container)

    def set_value(self, key, value):
        max_length = 36 
        if key in self.labels:
            # Проверяем, является ли значение числом
            if isinstance(value, int):
                value = f"{value:,}".replace(",", " ")  # Форматируем целое число
            elif isinstance(value, float):
                value = f"{value:,.2f}".replace(",", " ")  # Форматируем число с плавающей точкой
            elif isinstance(value, str):
                try:
                    # Пробуем преобразовать строку в число
                    if "." in value:
                        numeric_value = float(value.replace(" ", "").replace(",", ""))
                        value = f"{numeric_value:,.2f}".replace(",", " ")
                    else:
                        numeric_value = int(value.replace(" ", "").replace(",", ""))
                        value = f"{numeric_value:,}".replace(",", " ")
                except ValueError:
                    pass
            
            if key == "Победы" and value != '-':
                value = f"{value}%"
            elif key == "Проведено боев" and value != '-':
                wins = self.api_client.main_stats_structure["wins"]
                losses = self.api_client.main_stats_structure["losses"]
                value = f"{value} [{wins}-{losses}]"
            
            dots = '.' * max(1, max_length - len(key) - len(value))
            self.labels[key].setText(f"{key} {dots} {value}")  
    
    def update_stats_periodically(self):
        while True:
            try:
                success = self.api_client.set_main_stats()
                if success:
                    self.set_value("Золото", str(self.api_client.main_stats_structure["gold"]))
                    self.set_value("Кредиты", str(self.api_client.main_stats_structure["credits"]))
                    self.set_value("Боевой опыт", str(self.api_client.main_stats_structure["exp_battle"]))
                    self.set_value("Cвободный опыт", str(self.api_client.main_stats_structure["exp_free"]))
                    self.set_value("Проведено боев", str(self.api_client.main_stats_structure["battles"]))
                    self.stream_page.set_value("Бои", str(self.api_client.main_stats_structure["battles"] + self.api_client.rating_stats_structure["battles"]))
                    if self.api_client.main_stats_structure["battles"] > 0:
                        self.set_value("Победы", str(round((self.api_client.main_stats_structure["wins"] / self.api_client.main_stats_structure["battles"]) * 100.00, 2)))
                        self.set_value("Урон", str(self.api_client.main_stats_structure["totalDamage"] // self.api_client.main_stats_structure["battles"]))
                        self.set_value("Опыт", str(self.api_client.main_stats_structure["exp_battle"] // self.api_client.main_stats_structure["battles"]))
                    else:
                        self.set_value("Победы", "-")
                        self.set_value("Урон", "-")
                        self.set_value("Опыт", "-")

                    if self.api_client.main_stats_structure["battles"] > 0 or self.api_client.rating_stats_structure["battles"] > 0:
                        self.stream_page.set_value("Победы", str(round(((self.api_client.main_stats_structure["wins"] + self.api_client.rating_stats_structure["wins"]) / (self.api_client.main_stats_structure["battles"] + self.api_client.rating_stats_structure["battles"])) * 100.00, 2)))
                        self.stream_page.set_value("Урон", str((self.api_client.main_stats_structure["totalDamage"] + self.api_client.rating_stats_structure["totalDamage"]) // (self.api_client.main_stats_structure["battles"] + self.api_client.rating_stats_structure["battles"])))
                        self.stream_page.set_value("Опыт", str((self.api_client.main_stats_structure["exp_battle"] + self.api_client.rating_stats_structure["exp_battle"]) // (self.api_client.main_stats_structure["battles"] + self.api_client.rating_stats_structure["battles"])))
                    else:
                        self.stream_page.set_value("Победы", "-")
                        self.stream_page.set_value("Урон", "-")
                        self.stream_page.set_value("Опыт", "-")
                else:
                    print("Не удалось обновить статистику.")
            except Exception as e:
                print(f"Ошибка при обновлении статистики: {e}")

            time.sleep(30)

class Rating(QWidget): 
    def __init__(self, api_client, stream_page):
        super().__init__()
        self.api_client = api_client
        self.stream_page = stream_page
        self.setup_ui()

        self.updating_thread = threading.Thread(target=self.update_stats_periodically)
        self.updating_thread.daemon = True
        self.updating_thread.start()
    
    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(0)

        self.labels = {}
        self.add_icon_with_label(layout, resource_path('src/rating_icon.webp'), "Текущий рейтинг", is_top=True)
        self.add_icon_with_label(layout, resource_path('src/progress_icon.webp'), "Прогресс рейтинга")
        self.add_icon_with_label(layout, resource_path('src/calib_icon.webp'), "Калибровочные бои")

        container = QWidget()
        container.setStyleSheet("background-color: transparent;")
        h_layout = QHBoxLayout()
        h_layout.setContentsMargins(10, 2, 10, 2)
        h_layout.setSpacing(10)

        iconArrow = QLabel()
        iconArrow.setPixmap(QPixmap(resource_path('src/arrow_icon.webp')))
        iconArrow.setStyleSheet("background-color: rgba(30, 30, 30, 0);")

        avgLabel = QLabel(self)
        avgLabel.setText("Боевая эффективность (AVG)")
        avgLabel.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 14px;
            letter-spacing: 0px;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        avgLabel.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        avgLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        h_layout.addWidget(iconArrow)
        h_layout.addWidget(avgLabel)

        container.setLayout(h_layout)
        layout.addWidget(container)

        self.add_icon_with_label(layout, resource_path('src/battles_icon.webp'), "Проведено боев")
        self.add_icon_with_label(layout, resource_path('src/win_icon.webp'), "Победы")
        self.add_icon_with_label(layout, resource_path('src/damage_icon.webp'), "Урон")
        self.add_icon_with_label(layout, resource_path('src/xp_icon.webp'), "Опыт", is_bottom=True)

        self.full_height = self.sizeHint().height()

        # Пример: обновление значений
        self.set_value("Текущий рейтинг", "-")
        self.set_value("Прогресс рейтинга", "-")
        self.set_value("Калибровочные бои", "-")
        self.set_value("Проведено боев", "-")
        self.set_value("Победы", "-")
        self.set_value("Урон", "-")
        self.set_value("Опыт", "-")

    def add_icon_with_label(self, layout, icon_path, key, is_top=False, is_bottom=False):
        container = QWidget()
        h_layout = QHBoxLayout()
        h_layout.setContentsMargins(10, 2, 10, 2)
        h_layout.setSpacing(10)

        icon = QLabel()
        icon.setPixmap(QPixmap(icon_path))
        icon.setStyleSheet("background-color: rgba(30, 30, 30, 0);")

        label = QLabel()

        label.setStyleSheet("""
            font-family: Consolas;
            font-size: 12px;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        label.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)

        self.labels[key] = label

        h_layout.addWidget(icon)
        h_layout.addWidget(label)

        container.setLayout(h_layout)

        radius_style = ""
        if is_top:
            radius_style += "border-top-right-radius: 10px;"
        if is_bottom:
            radius_style += "border-bottom-right-radius: 10px;"

        container.setStyleSheet(f"""
            background-color: rgba(30, 30, 30, 0);
            {radius_style}
        """)

        layout.addWidget(container)

    def set_value(self, key, value):
        max_length = 36
        if key in self.labels:
            if isinstance(value, int):
                value = f"{value:,}".replace(",", " ")
            elif isinstance(value, float):
                value = f"{value:,.2f}".replace(",", " ")
            elif isinstance(value, str):
                try:
                    if "." in value:
                        numeric_value = float(value.replace(" ", "").replace(",", ""))
                        value = f"{numeric_value:,.2f}".replace(",", " ")
                    else:
                        numeric_value = int(value.replace(" ", "").replace(",", ""))
                        value = f"{numeric_value:,}".replace(",", " ")
                except ValueError:
                    pass

            if key == "Победы" and value != '-':
                value = f"{value}%"
            elif key == "Проведено боев" and value != '-':
                wins = self.api_client.rating_stats_structure["wins"]
                losses = self.api_client.rating_stats_structure["battles"] - self.api_client.rating_stats_structure["wins"]
                value = f"{value} [{wins}-{losses}]"
            dots = '.' * max(1, max_length - len(key) - len(value))

            if key == "Прогресс рейтинга" and value != '-':
                if(value[0] != '-'):
                    value = f"+{value}"
            dots = '.' * max(1, max_length - len(key) - len(value))
            self.labels[key].setText(f"{key} {dots} {value}")  
    
    def update_stats_periodically(self):
        while True:
            try:
                success = self.api_client.set_rating_stats()
                if success:
                    self.set_value("Текущий рейтинг", str(self.api_client.rating_stats_structure["mm_rating"]))
                    self.set_value("Прогресс рейтинга", str(self.api_client.rating_stats_structure["mm_rating"] - self.api_client.first_rating_stats_structure["mm_rating"]))
                    self.set_value("Калибровочные бои", str(self.api_client.rating_stats_structure["calib_battle"]))
                    self.set_value("Проведено боев", str(self.api_client.rating_stats_structure["battles"]))
                    if self.api_client.rating_stats_structure["battles"] > 0:
                        self.set_value("Победы", str(round((self.api_client.rating_stats_structure["wins"] / self.api_client.rating_stats_structure["battles"]) * 100.00, 2)))
                        self.set_value("Урон", str(self.api_client.rating_stats_structure["totalDamage"] // self.api_client.rating_stats_structure["battles"]))
                        self.set_value("Опыт", str(self.api_client.rating_stats_structure["exp_battle"] // self.api_client.rating_stats_structure["battles"]))
                    else:
                        self.set_value("Победы", "-")
                        self.set_value("Урон", "-")
                        self.set_value("Опыт", "-")

                    if self.api_client.rating_stats_structure["mm_rating"] - self.api_client.first_rating_stats_structure["mm_rating"] >= 0:
                        self.stream_page.set_value("Рейтинг", str(self.api_client.rating_stats_structure["mm_rating"]) + " (+" + str(self.api_client.rating_stats_structure["mm_rating"] - self.api_client.first_rating_stats_structure["mm_rating"]) + ")")
                    else:
                        self.stream_page.set_value("Рейтинг", str(self.api_client.rating_stats_structure["mm_rating"]) + " (" + str(self.api_client.rating_stats_structure["mm_rating"] - self.api_client.first_rating_stats_structure["mm_rating"]) + ")")
                else:
                    print("Не удалось обновить статистику.")
            except Exception as e:
                print(f"Ошибка при обновлении статистики: {e}")

            time.sleep(30)  

class TanksStat(QWidget):   
    update_tank_row = pyqtSignal(int, str, str, str, str)
    reset_tank_data = pyqtSignal()

    def __init__(self, api_client):
        super().__init__()
        self.api_client = api_client
        self.COLUMN_WIDTHS = {
            'tank': 120,
            'battles': 50,
            'wins': 50,
            'damage': 60
        }

        self.update_tank_row.connect(self.addTankRow)
        self.reset_tank_data.connect(self.clear_tank_data)
        self.is_dataset = True

        self.updating_thread = threading.Thread(target=self.update_tech_stats_periodically)
        self.updating_thread.daemon = True
        self.updating_thread.start()
        self.setup_ui()
    
    def setup_ui(self):
        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(5)
        main_layout.setContentsMargins(10, 2, 10, 2)
        main_layout.setAlignment(Qt.AlignTop)

        title_layout = QHBoxLayout()
        title_layout.setSpacing(10)
        title_layout.setContentsMargins(10, 2, 10, 2)
        
        arrow_icon = QLabel()
        arrow_icon.setPixmap(QPixmap(resource_path('src/arrow_icon.webp')))
        arrow_icon.setStyleSheet("background-color: transparent;")
        title_layout.addWidget(arrow_icon)
        
        title = QLabel("Статистика по технике")
        title.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 14px;
            color: #e2ded3;
            background-color: transparent;
        """)
        title.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        title.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        title_layout.addWidget(title)
        main_layout.addLayout(title_layout)

        header_widget = QWidget()
        header_widget.setStyleSheet("""
            background-color: #383838;
            border-radius: 3px;
        """)
        header_widget.setFixedHeight(25)
        header_layout = QHBoxLayout(header_widget)
        header_layout.setSpacing(0)
        header_layout.setContentsMargins(0, 0, 0, 0)

        for col_type, icon_path in [('tank', 'src/tanks_icon.webp'),
                                ('battles', 'src/battles_icon.webp'),
                                ('wins', 'src/win_icon.webp'),
                                ('damage', 'src/damage_icon.webp')]:
            icon = QLabel()
            icon.setPixmap(QPixmap(resource_path(icon_path)))
            icon.setStyleSheet("""
                background-color: transparent;
            """)
            icon.setFixedWidth(self.COLUMN_WIDTHS[col_type])
            icon.setAlignment(Qt.AlignCenter)
            header_layout.addWidget(icon)

        main_layout.addWidget(header_widget)

        self.data_grid = QGridLayout()
        self.data_grid.setHorizontalSpacing(0)
        self.data_grid.setVerticalSpacing(1)
        self.data_grid.setContentsMargins(0, 0, 0, 0)
        self.data_grid.setAlignment(Qt.AlignTop)

        if self.data_grid.rowCount() > 7:
            spacer = QWidget()
            spacer.setFixedWidth(0)
            self.header_spacer = spacer
            header_layout.addWidget(spacer)
        
        data_widget = QWidget()
        data_widget.setLayout(self.data_grid)
        data_widget.setStyleSheet("""
            background-color: rgba(0, 0, 0, 0);  /* Прозрачный фон */
        """)

        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setWidget(data_widget)
        scroll_area.setViewportMargins(0, 0, 2, 0)
        scroll_area.setStyleSheet("""
            QScrollArea {
                background-color: rgba(0, 0, 0, 0);
                border: none;
                border-radius: 0px;
            }
            QScrollBar:vertical {
                background: rgba(0, 0, 0, 0);
                width: 4px;
                margin: 2px 0;
                border-radius: 0px;
            }
            QScrollBar::handle:vertical {
                background: #555555;
                border-radius: 2px;
            }
            QScrollBar::groove:vertical {
                border-radius: 0px;
            }
            QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
                height: 0;
            }
        """)
        scroll_area.setMaximumHeight(220)
        main_layout.addWidget(scroll_area)

    def addTankRow(self, row_num, tank_name, battles, wins, damage):
        row_widget = QWidget()
        row_widget.setStyleSheet("""
            background-color: #383838;
            border-radius: 3px;
        """)
        row_widget.setFixedHeight(30)
        row_layout = QHBoxLayout(row_widget)
        row_layout.setSpacing(0)
        row_layout.setContentsMargins(0, 0, 0, 0)
        
        tank_label = QLabel(str(tank_name))
        tank_label.setStyleSheet(self.getTextStyle() + "text-align: left;")
        tank_label.setFixedWidth(self.COLUMN_WIDTHS['tank'])
        tank_label.setFixedHeight(25)
        row_layout.addWidget(tank_label)

        data = [
            (str(battles), 'battles'),
            (str(wins), 'wins'),
            (str(damage), 'damage')
        ]
        
        for text, col_type in data:
            label = QLabel(text)
            style = self.getTextStyle()
            
            if col_type == 'wins':
                try:
                    win_percent = float(text.strip('%'))
                    if win_percent >= 70.00:
                        color = "#9989e6"
                    elif 60.00 <= win_percent < 70.00:
                        color = "#72d1ff"
                    elif 50.00 <= win_percent < 60.00:
                        color = "#a8e689"
                    else:
                        color = "#ffffff"
                    style += f"color: {color}; font-weight: bold;"
                except:
                    pass
            
            label.setStyleSheet(style)
            label.setFixedWidth(self.COLUMN_WIDTHS[col_type])
            label.setFixedHeight(25)
            label.setAlignment(Qt.AlignCenter)
            row_layout.addWidget(label)
        
        self.data_grid.addWidget(row_widget, row_num, 0, 1, 4)

    def getTextStyle(self):
        return """
            font-family: Consolas;
            font-size: 12px;
            color: #e2ded3;
            background-color: #383838;
            padding: 5px 0px;
            margin: 0px;
            border: none;
        """
    
    def update_tech_stats_periodically(self):
        while True:
            try:
                success = self.api_client.set_tech_stats()
                if success:
                    row_index = 0
                    for tank_id, current_stats in self.api_client.tech_stats_array.items():

                        prev_stats = self.api_client.prev_tech_stats_array.get(tank_id, {})
                        prev_battles = prev_stats.get("battles", 0)
                        current_battles = current_stats.get("battles", 0)

                        if current_battles > prev_battles:

                            if int(tank_id) not in self.api_client.tech_info_dataset:
                                print(f"Танк ID {tank_id}: информация отсутствует в tech_info_dataset, загружаем...")
                                self.api_client.set_tech_info(tank_id)
                                print(f"Проверка после загрузки: {self.api_client.tech_info_dataset.get(tank_id)}")
                                time.sleep(0.1)

                            if int(tank_id) not in self.api_client.tech_info_dataset:
                                print(f"Ошибка: Танк с ID {tank_id} отсутствует в tech_info_dataset после загрузки.")
                                continue

                            tank_name = self.api_client.tech_info_dataset[tank_id].get("name")
                            battles = str(current_battles)
                            if current_battles > 0:
                                wins = str(round(
                                    (current_stats.get("wins", 0) / current_battles) * 100.00, 2)) + "%"
                                damage = str(current_stats.get("totalDamage", 0) // current_battles)
                            else:
                                wins = "-"
                                damage = "-"


                            existing_row = self.get_row_by_tank_id(tank_id)
                            if existing_row is not None:
                                self.update_existing_row(existing_row, tank_name, battles, wins, damage)
                            else:
                                new_row_index = self.data_grid.rowCount()
                                self.update_tank_row.emit(new_row_index, tank_name, battles, wins, damage)

                            self.api_client.prev_tech_stats_array[tank_id] = current_stats
                else:
                    print("Не удалось обновить статистику.")
            except Exception as e:
                print(f"Ошибка при обновлении статистики: {e}")

            time.sleep(30)
    
    def get_row_by_tank_id(self, tank_id):
        tank_name = self.api_client.tech_info_dataset[tank_id].get("name", "Неизвестно")
        print(f"Ищем строку для танка: {tank_name} (ID: {tank_id})")  # Лог начала поиска
        for row in range(self.data_grid.rowCount()):
            item = self.data_grid.itemAtPosition(row, 0)  # Получаем виджет из первой колонки
            if item:
                row_widget = item.widget()  # Получаем row_widget
                if row_widget is None:
                    print(f"Строка {row}: виджет отсутствует.")
                    continue
                # Ищем QLabel внутри row_widget
                tank_label = row_widget.findChild(QLabel)
                if tank_label:
                    label_text = tank_label.text().strip()  # Удаляем лишние пробелы
                    print(f"Проверка строки {row}: {label_text} == {tank_name}")  # Отладочный вывод
                    if label_text == tank_name:
                        print(f"Строка найдена для танка {tank_name} на позиции {row}.")
                        return row
                else:
                    print(f"Строка {row}: QLabel не найден внутри row_widget.")
        print(f"Строка для танка {tank_name} не найдена.")
        return None
    
    def update_existing_row(self, row_index, tank_name, battles, wins, damage):
        row_widget = self.data_grid.itemAtPosition(row_index, 0).widget()
        if not row_widget:
            print(f"Ошибка: строка с индексом {row_index} не найдена.")
            return
    
        # Обновляем данные в строке
        labels = row_widget.findChildren(QLabel)  # Находим все QLabel в строке
        if len(labels) < 4:
            print(f"Ошибка: недостаточно столбцов для обновления строки с индексом {row_index}.")
            return
    
        # Обновляем значения в столбцах
        labels[0].setText(tank_name)  # Имя танка
        labels[1].setText(battles)   # Количество боёв
        win_style = self.getTextStyle()
        try:
            win_percent = float(wins.strip('%'))
            if win_percent >= 70.00:
                color = "#9989e6"
            elif 60.00 <= win_percent < 70.00:
                color = "#72d1ff"
            elif 50.00 <= win_percent < 60.00:
                color = "#a8e689"
            else:
                color = "#ffffff"
            win_style += f"color: {color}; font-weight: bold;"
        except:
            pass
        labels[2].setText(wins)  # Победы
        labels[2].setStyleSheet(win_style)      # Победы
        labels[3].setText(damage)    # Урон
    
        print(f"Строка с индексом {row_index} обновлена: {tank_name}, {battles}, {wins}, {damage}")

    def clear_tank_data(self):
        while self.data_grid.count():
            item = self.data_grid.takeAt(0)
            widget = item.widget()
            if widget is not None:
                widget.deleteLater()

class Graphics(QWidget):   
    def __init__(self, api_client):
        super().__init__()
        self.api_client = api_client
        self.stats = self.api_client.graphics_value
        self.prev_battles = -1
        self.prev_rating_battles = 0
        self.graph_widgets = {}
        self.setStyleSheet("background: transparent;")
        self.setup_ui()

        self.updating_thread = threading.Thread(target=self.update_grapgics_periodically)
        self.updating_thread.daemon = True
        self.updating_thread.start()

    
    def setup_ui(self):
        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(5)
        main_layout.setContentsMargins(10, 2, 10, 2)
        main_layout.setAlignment(Qt.AlignTop)

        title_layout = QHBoxLayout()
        title_layout.setSpacing(10)
        title_layout.setContentsMargins(10, 2, 10, 2)
        
        arrow_icon = QLabel()
        arrow_icon.setPixmap(QPixmap(resource_path('src/arrow_icon.webp')))
        arrow_icon.setStyleSheet("background-color: transparent;")
        title_layout.addWidget(arrow_icon)
        
        title = QLabel("График статистики")
        title.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 14px;
            color: #e2ded3;
            background-color: transparent;
        """)
        title.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        title.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        title_layout.addWidget(title)
        main_layout.addLayout(title_layout)

        header_widget = QWidget()
        header_widget.setStyleSheet("""
            background-color: #383838;
            border-radius: 3px;
        """)
        header_widget.setFixedHeight(25)
        header_layout = QHBoxLayout(header_widget)
        header_layout.setSpacing(0)
        header_layout.setContentsMargins(0, 0, 0, 0)

        for index, (col_type, icon_path) in enumerate([
            ('wins', 'src/win_icon.webp'),
            ('damage', 'src/damage_icon.webp'),
            ('xp', 'src/xp_icon.webp'),
            ('rating', 'src/rating_icon.webp')
        ]):
            button = QPushButton()
            button.setIcon(QIcon(QPixmap(resource_path(icon_path))))
            button.setIconSize(QSize(24, 24))
            button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Preferred)
            button.setStyleSheet("""
                QPushButton {
                    background-color: transparent;
                    border: none;
                }
                QPushButton:hover {
                    background-color: rgba(255, 255, 255, 0.1);
                }
                QPushButton:pressed {
                    background-color: rgba(255, 255, 255, 0.2);
                }
            """)
            button.setCursor(Qt.PointingHandCursor)
            button.clicked.connect(lambda checked, i=index: self.switch_graphics(i))
            header_layout.addWidget(button)

        main_layout.addWidget(header_widget)

        self.stacked_graphics = QStackedWidget()
        self.stacked_graphics.setMaximumHeight(200)
        self.stacked_graphics.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        main_layout.addWidget(self.stacked_graphics)

        # Добавляем 4 "графика" (замени на реальные виджеты с графиками)

        for title, data in self.stats.items():
            chart = SimplePlotWidget(title, data)
            self.graph_widgets[title] = chart

            container = QWidget()
            container_layout = QVBoxLayout(container)
            container_layout.setContentsMargins(0, 0, 0, 0)
            container_layout.setAlignment(Qt.AlignCenter)
            container_layout.addWidget(chart)
    
            self.stacked_graphics.addWidget(container)

        # По умолчанию показываем первый график
        self.stacked_graphics.setCurrentIndex(0)


    def switch_graphics(self, index):
        self.stacked_graphics.setCurrentIndex(index)

    #функия переодического добавления точек в график и в api_client.graphics_value
    def update_grapgics_periodically(self):
        while True:
            for key in self.stats.keys():
                if key not in self.api_client.graphics_value:
                    self.api_client.graphics_value[key] = []
                    self.api_client.graphics_x_value[key] = []

                if not self.api_client.graphics_value[key]:
                    self.api_client.graphics_value[key].append(0.0)
                    self.api_client.graphics_x_value[key].append(0.0)
                    if self.api_client.graphics_value["damage"] and self.api_client.graphics_value["damage"][-1] > 0:
                        if self.api_client.graphics_value["wins"]:
                            self.add_value_to_graphics("wins", self.api_client.graphics_value["wins"][-1])
                        if self.api_client.graphics_value["damage"]:
                            self.add_value_to_graphics("damage", self.api_client.graphics_value["damage"][-1])
                        if self.api_client.graphics_value["xp"]:
                            self.add_value_to_graphics("xp", self.api_client.graphics_value["xp"][-1])
                        if self.api_client.graphics_value["rating"]:
                            self.add_value_to_graphics("rating", self.api_client.graphics_value["rating"][-1])

            #нужно сравнить текущее кол-во боев (обычные + рейтинг) с prev_battles, если больше, то добавляем в графики
            current_battles = self.api_client.main_stats_structure["battles"] + self.api_client.rating_stats_structure["battles"]
            if current_battles <= self.prev_battles or current_battles == 0:
                time.sleep(30)
                continue

            self.prev_battles = current_battles
                
            # Добавляем новые значения в api_client.graphics_value
            
            wins = self.api_client.main_stats_structure["wins"] + self.api_client.rating_stats_structure["wins"]
            self.api_client.graphics_value["wins"].append(round((wins / current_battles) * 100.00, 2))
            self.api_client.graphics_x_value["wins"].append(current_battles)
            self.add_value_to_graphics("wins", self.api_client.graphics_value["wins"][-1])
            
            damage = (self.api_client.main_stats_structure["totalDamage"] + self.api_client.rating_stats_structure["totalDamage"]) // current_battles
            self.api_client.graphics_value["damage"].append(damage)
            self.api_client.graphics_x_value["damage"].append(current_battles)
            self.add_value_to_graphics("damage", self.api_client.graphics_value["damage"][-1])

            xp = (self.api_client.main_stats_structure["exp_battle"] + self.api_client.rating_stats_structure["exp_battle"]) // current_battles
            self.api_client.graphics_value["xp"].append(xp)
            self.api_client.graphics_x_value["xp"].append(current_battles)
            self.add_value_to_graphics("xp", self.api_client.graphics_value["xp"][-1])

            rating = self.api_client.rating_stats_structure["mm_rating"]
            current_rating_battle = self.api_client.rating_stats_structure["battles"]
            if current_rating_battle > self.prev_rating_battles:
                self.prev_rating_battles = current_rating_battle
                self.api_client.graphics_value["rating"].append(rating)
                self.api_client.graphics_x_value["rating"].append(current_rating_battle)
                self.add_value_to_graphics("rating", self.api_client.graphics_value["rating"][-1])

            for key, value in self.api_client.graphics_value.items():
                if key in self.stats:
                    self.stats[key] = value.copy()
                else:
                    print(f"Ключ {key} не найден в stats.")
            time.sleep(30)

    def add_value_to_graphics(self, key, value):
        widget = self.graph_widgets.get(key)
        if widget:
            data = self.api_client.graphics_value[key]
            if not data:
                return
            widget.ax.clear()

            x = self.api_client.graphics_x_value[key]
            y = self.api_client.graphics_value[key]

            if key == "wins":
                win_percent = y[-1]  # последнее значение
                if win_percent >= 70.00:
                    color = "#9989e6"
                elif 60.00 <= win_percent < 70.00:
                    color = "#72d1ff"
                elif 50.00 <= win_percent < 60.00:
                    color = "#a8e689"
                else:
                    color = "#ffffff"
            else:
                color = "#e2ded3"

            widget.ax.plot(x, y, marker='o', color=color)
            widget.ax.fill_between(x, y, color=color, alpha=0.2)
            widget.ax.grid(True, color='gray', linestyle='--', alpha=0.3)

            for label in widget.ax.get_xticklabels() + widget.ax.get_yticklabels():
                label.set_fontname("Consolas")
                label.set_fontsize(9)
                label.set_color("#e2ded3")

            widget.ax.set_facecolor('none')
            widget.ax.tick_params(colors='#e2ded3')
            for spine in widget.ax.spines.values():
                spine.set_visible(False)

            widget.fig.subplots_adjust(left=0.12, right=1.00, top=1.00, bottom=0.12)
            widget.draw()
        else:
            print(f"График для ключа '{key}' не найден.")

class Other(QWidget):   
    def __init__(self, api_client, stream_page):
        super().__init__()
        self.api_client = api_client
        self.stream_page = stream_page
        self.setup_ui()

        self.updating_thread = threading.Thread(target=self.update_other_periodically)
        self.updating_thread.daemon = True
        self.updating_thread.start()

        self.updating_thread_master = threading.Thread(target=self.update_masters_periodically)
        self.updating_thread_master.daemon = True
        self.updating_thread_master.start()
    
    def setup_ui(self):
        layout = QVBoxLayout(self)
        layout.setSpacing(0)

        main_mastery_container = QWidget()
        main_mastery_container.setStyleSheet("""
            border-radius: 10px;
            background-color: rgba(30, 30, 30, 0);
            border: 1px solid rgba(57, 57, 57, 255);
        """)

        mastery_layout = QVBoxLayout()
        mastery_layout.setSpacing(0)

        mastery_container = QWidget()
        mastery_container.setStyleSheet("""
            background-color: rgba(57, 57, 57, 0);
            border: none;
        """)
        
        medal_layout = QHBoxLayout()
        medal_layout.setContentsMargins(10, 2, 10, 2)
        medal_layout.setSpacing(20)
        medal_layout.setAlignment(Qt.AlignCenter)

        iconMaster = QLabel()
        iconMaster.setPixmap(QPixmap(resource_path('src/master_icon.png')))
        iconMaster.setStyleSheet("background-color: rgba(30, 30, 30, 0); border: none;")
        medal_layout.addWidget(iconMaster)

        iconMaster_1 = QLabel()
        iconMaster_1.setPixmap(QPixmap(resource_path('src/master1_icon.png')))
        iconMaster_1.setStyleSheet("background-color: rgba(30, 30, 30, 0); border: none;")
        medal_layout.addWidget(iconMaster_1)

        iconMaster_2 = QLabel()
        iconMaster_2.setPixmap(QPixmap(resource_path('src/master2_icon.png')))
        iconMaster_2.setStyleSheet("background-color: rgba(30, 30, 30, 0); border: none;")
        medal_layout.addWidget(iconMaster_2)

        iconMaster_3 = QLabel()
        iconMaster_3.setPixmap(QPixmap(resource_path('src/master3_icon.png')))
        iconMaster_3.setStyleSheet("background-color: rgba(30, 30, 30, 0); border: none;")
        medal_layout.addWidget(iconMaster_3)

        mastery_container.setLayout(medal_layout)
        mastery_layout.addWidget(mastery_container)

        mastery_value_container = QWidget()
        mastery_value_container.setStyleSheet("""
            background-color: rgba(57, 57, 57, 0);
            border: none;
        """)

        medal_value_layout = QHBoxLayout()
        medal_value_layout.setContentsMargins(10, 2, 10, 5)
        medal_value_layout.setSpacing(20)
        medal_value_layout.setAlignment(Qt.AlignCenter)

        label_master = QLabel(self)
        label_master.setObjectName("master_label")
        label_master.setFixedSize(50, 20)
        label_master.setText("-")
        label_master.setAlignment(Qt.AlignCenter)
        label_master.setStyleSheet("""
            font-family: Consolas;
            font-size: 16px;
            font-weight: bold;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        medal_value_layout.addWidget(label_master)


        label_master_1 = QLabel(self)
        label_master_1.setObjectName("master_label_1")
        label_master_1.setFixedSize(50, 20)
        label_master_1.setText("-")
        label_master_1.setAlignment(Qt.AlignCenter)
        label_master_1.setStyleSheet("""
            font-family: Consolas;
            font-size: 16px;
            font-weight: bold;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        medal_value_layout.addWidget(label_master_1)


        label_master_2 = QLabel(self)
        label_master_2.setObjectName("master_label_2")
        label_master_2.setFixedSize(50, 20)
        label_master_2.setText("-")
        label_master_2.setAlignment(Qt.AlignCenter)
        label_master_2.setStyleSheet("""
            font-family: Consolas;
            font-size: 16px;
            font-weight: bold;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        medal_value_layout.addWidget(label_master_2)


        label_master_3 = QLabel(self)
        label_master_3.setObjectName("master_label_3")
        label_master_3.setFixedSize(50, 20)
        label_master_3.setText("-")
        label_master_3.setAlignment(Qt.AlignCenter)
        label_master_3.setStyleSheet("""
            font-family: Consolas;
            font-size: 16px;
            font-weight: bold;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        medal_value_layout.addWidget(label_master_3)

        mastery_value_container.setLayout(medal_value_layout)
        mastery_layout.addWidget(mastery_value_container)
        main_mastery_container.setLayout(mastery_layout)
        layout.addWidget(main_mastery_container)

        container = QWidget()
        container.setStyleSheet("background-color: transparent;")
        h_layout = QHBoxLayout()
        h_layout.setContentsMargins(10, 2, 10, 2)
        h_layout.setSpacing(10)

        iconArrow = QLabel()
        iconArrow.setPixmap(QPixmap(resource_path('src/arrow_icon.webp')))
        iconArrow.setStyleSheet("background-color: rgba(30, 30, 30, 0);")

        avgLabel = QLabel(self)
        avgLabel.setText("Боевые показатели")
        avgLabel.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 14px;
            letter-spacing: 0px;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        avgLabel.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        avgLabel.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        h_layout.addWidget(iconArrow)
        h_layout.addWidget(avgLabel)

        container.setLayout(h_layout)
        layout.addWidget(container)

        self.labels = {}
        self.add_icon_with_label(layout, resource_path('src/percent_damage_icon.png'), "Процент попаданий", is_top=True)
        self.add_icon_with_label(layout, resource_path('src/percent_leave_icon.png'), "Процент выживания")
        self.add_icon_with_label(layout, resource_path('src/time_icon.png'), "Ср. время выживания")
        self.add_icon_with_label(layout, resource_path('src/k_shot_icon.png'), "Коэффициент урона")
        self.add_icon_with_label(layout, resource_path('src/k_tank_icon.png'), "Коэффициент уничтожения", is_bottom=True)

        self.full_height = self.sizeHint().height()

        self.set_value("Процент попаданий", "-")
        self.set_value("Процент выживания", "-")
        self.set_value("Ср. время выживания", "-")
        self.set_value("Коэффициент урона", "-")
        self.set_value("Коэффициент уничтожения", "-")

        self.labels["master_label"] = label_master
        self.labels["master_label_1"] = label_master_1
        self.labels["master_label_2"] = label_master_2
        self.labels["master_label_3"] = label_master_3

    def add_icon_with_label(self, layout, icon_path, key, is_top=False, is_bottom=False):
        container = QWidget()
        h_layout = QHBoxLayout()
        h_layout.setContentsMargins(10, 2, 10, 2)
        h_layout.setSpacing(10)

        icon = QLabel()
        icon.setPixmap(QPixmap(icon_path))
        icon.setStyleSheet("background-color: rgba(30, 30, 30, 0);")

        label = QLabel()

        label.setStyleSheet("""
            font-family: Consolas;
            font-size: 12px;
            color: #e2ded3;
            background-color: rgba(30, 30, 30, 0);
        """)
        label.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)

        self.labels[key] = label

        h_layout.addWidget(icon)
        h_layout.addWidget(label)

        container.setLayout(h_layout)

        radius_style = ""
        if is_top:
            radius_style += "border-top-right-radius: 10px;"
        if is_bottom:
            radius_style += "border-bottom-right-radius: 10px;"

        container.setStyleSheet(f"""
            background-color: rgba(30, 30, 30, 0);
            {radius_style}
        """)

        layout.addWidget(container)

    def set_value(self, key, value):
        max_length = 36 
        if key in self.labels:
            if key not in ["Ср. время выживания"]:  # Исключаем ключи, которые не нужно форматировать
                # Проверяем, является ли значение числом
                if isinstance(value, int):
                    value = f"{value:,}".replace(",", " ")  # Форматируем целое число
                elif isinstance(value, float):
                    value = f"{value:,.2f}".replace(",", " ")  # Форматируем число с плавающей точкой
                elif isinstance(value, str):
                    try:
                        # Пробуем преобразовать строку в число
                        if "." in value:
                            numeric_value = float(value.replace(" ", "").replace(",", ""))
                            value = f"{numeric_value:,.2f}".replace(",", " ")
                        else:
                            numeric_value = int(value.replace(" ", "").replace(",", ""))
                            value = f"{numeric_value:,}".replace(",", " ")
                    except ValueError:
                        pass

            if key == "Процент попаданий" and value != '-':
                value = f"{value}%"
            elif key == "Процент выживания" and value != '-':
                value = f"{value}%"
            dots = '.' * max(1, max_length - len(key) - len(value))
            self.labels[key].setText(f"{key} {dots} {value}")  

    def set_value_masters(self, key, value):
        if key in self.labels:
            self.labels[key].setText(f"{value}")

    def convert_seconds_to_minutes_seconds(self, seconds):
        td = timedelta(seconds=seconds)
        return f"{td.seconds // 60}:{td.seconds % 60:02}"
    
    def update_other_periodically(self):
        while True:
            try:
                success = self.api_client.set_other_stats()
                if success:
                    if self.api_client.other_stats_structure["shots"] > 0:
                        self.set_value("Процент попаданий", str(round((self.api_client.other_stats_structure["hits"] / self.api_client.other_stats_structure["shots"]) * 100.00, 2)))
                        self.stream_page.set_value("Точность", str(round((self.api_client.other_stats_structure["hits"] / self.api_client.other_stats_structure["shots"]) * 100.00, 2)))
                    if self.api_client.other_stats_structure["battles"] > 0:
                        self.set_value("Процент выживания", str(round((self.api_client.other_stats_structure["survived"] / self.api_client.other_stats_structure["battles"]) * 100.00, 2)))
                        self.set_value("Ср. время выживания", self.convert_seconds_to_minutes_seconds(self.api_client.other_stats_structure["lifeTime"] // self.api_client.other_stats_structure["battles"]))
                        self.set_value("Коэффициент урона", str(round((self.api_client.other_stats_structure["totalDamage"] / self.api_client.other_stats_structure["receiverDamage"]), 2)))

                        deaths = self.api_client.other_stats_structure["battles"] - self.api_client.other_stats_structure["survived"]
                        if deaths > 0:
                            kill_death_ratio = round(self.api_client.other_stats_structure["frags"] / deaths, 2)
                        else:
                            kill_death_ratio = self.api_client.other_stats_structure["frags"]
                        self.set_value("Коэффициент уничтожения", str(kill_death_ratio))

                        self.stream_page.set_value("Выжил", str(round((self.api_client.other_stats_structure["survived"] / self.api_client.other_stats_structure["battles"]) * 100.00, 2)))
                else:
                    print("Не удалось обновить статистику.")
            except Exception as e:
                print(f"Ошибка при обновлении статистики: {e}")

            time.sleep(30)

    def update_masters_periodically(self):
        while True:
            try:
                success = self.api_client.set_master_stats()
                if success:
                    self.set_value_masters("master_label", str(self.api_client.master_structure["mastery"]))
                    self.set_value_masters("master_label_1", str(self.api_client.master_structure["mastery_1"]))
                    self.set_value_masters("master_label_2", str(self.api_client.master_structure["mastery_2"]))
                    self.set_value_masters("master_label_3", str(self.api_client.master_structure["mastery_3"]))
                else:
                    print("Не удалось обновить статистику мастеров.")
            except Exception as e:
                print(f"Ошибка при обновлении статистики мастеров: {e}")

            time.sleep(30)

class Stream(QWidget):   
    def __init__(self, api_client, config_container, stacked_widget):
        super().__init__()
        self.api_client = api_client
        self.config_container = config_container
        self.stacked_widget = stacked_widget
        self.config_visible = True
        self.all_keys = ["Бои", "Победы", "Урон", "Опыт", "Выжил", "Точность", "Рейтинг", "Мастер"]
        self.visible_tiles = set(self.all_keys)
        self.tiles = {}
        self.setup_ui()
    
    def setup_ui(self):
        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(0)
        main_layout.setContentsMargins(10, 0, 10, 0)
        main_layout.setAlignment(Qt.AlignLeft | Qt.AlignTop)

        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setViewportMargins(0, 0, 0, 0)
        scroll_area.setLayoutDirection(Qt.LeftToRight)
        scroll_area.setStyleSheet("""
            QScrollArea {
                background-color: rgba(50, 50, 50, 0);
                border: none;
                border-radius: 5px;
            }
            QScrollBar:vertical {
                background: rgba(0, 0, 0, 0);
                width: 4px;
                margin: 2px 0;
                border-radius: 0px;
            }
            QScrollBar::handle:vertical {
                background: #555555;
                border-radius: 2px;
            }
            QScrollBar::groove:vertical {
                border-radius: 0px;
            }
            /* Горизонтальный скроллбар */
            QScrollBar:horizontal {
                background: rgba(0, 0, 0, 0);
                height: 4px;
                margin: 0 2px;
                border-radius: 0px;
            }
            QScrollBar::handle:horizontal {
                background: #555555;
                border-radius: 2px;
            }
            QScrollBar::groove:horizontal {
                border-radius: 0px;
            }
            /* Общие настройки */
            QScrollBar::add-line:vertical, 
            QScrollBar::sub-line:vertical,
            QScrollBar::add-line:horizontal, 
            QScrollBar::sub-line:horizontal {
                height: 0;
                width: 0;
            }
        """)
        scroll_area.setMaximumHeight(280)
        scroll_area.setMaximumWidth(250)
        scroll_area.setAlignment(Qt.AlignHCenter | Qt.AlignTop)


        self.data_grid = QGridLayout()
        self.data_grid.setHorizontalSpacing(0)
        self.data_grid.setVerticalSpacing(0)
        self.data_grid.setContentsMargins(0, 0, 0, 0)
        self.data_grid.setAlignment(Qt.AlignTop)

        data_widget = QWidget()
        data_widget.setLayout(self.data_grid)
        data_widget.setStyleSheet("""
            background-color: rgba(40, 40, 40, 0);  /* Прозрачный фон */
        """)
        data_widget.setSizePolicy(QSizePolicy.Minimum, QSizePolicy.Preferred)

        scroll_area.setWidget(data_widget)
        main_layout.addWidget(scroll_area)
        self.rebuild_grid()

    def add_label_and_value(self, grid, key, value, row, col):
        tile_container = QWidget()
        tile_container.setStyleSheet("""
            background-color: rgba(57, 57, 57, 150);
            border-radius: 5px;
            margin: 5px;
        """)
        tile_container.setFixedSize(125, 60)

        v_layout = QVBoxLayout(tile_container)
        v_layout.setContentsMargins(5, 5, 5, 5)
        v_layout.setSpacing(2)
        v_layout.setAlignment(Qt.AlignCenter)

        label = QLabel(key)
        label.setObjectName(key)
        label.setStyleSheet("""
            font-family: Consolas;
            font-size: 16px;
            font-weight: bold;
            color: #e2ded3;
            background-color: transparent;
        """)
        label.setAlignment(Qt.AlignCenter)
        v_layout.addWidget(label)

        if key == "Победы" or key == "Выжил" or key == "Точность":
            if value != "-":
                value = f"{value}%"
            else:
                value = f"{value}"
                
        value_label = QLabel(value)
        value_label.setObjectName(f"{key}_value")
        value_label.setStyleSheet("""
            font-family: Consolas;
            font-size: 16px;
            font-weight: bold;
            color: #e2ded3;
            background-color: transparent;
        """)
        value_label.setAlignment(Qt.AlignCenter)
        v_layout.addWidget(value_label)

        if key == "Победы" and value != '-':
            try:
                win_percent = float(value.strip('%'))
                if win_percent >= 70.00:
                    color = "#9989e6"
                elif 60.00 <= win_percent < 70.00:
                    color = "#72d1ff"
                elif 50.00 <= win_percent < 60.00:
                    color = "#a8e689"
                else:
                    color = "#ffffff"
                value_label.setStyleSheet(f"""
                    font-family: Consolas;
                    font-size: 16px;
                    font-weight: bold;
                    color: {color};
                    background-color: transparent;
                """)
            except ValueError:
                pass  

        self.tiles[key] = tile_container
        grid.addWidget(tile_container, row, col)

    def set_value(self, key, value):
        # Форматируем значение
        if isinstance(value, int):
            value = f"{value:,}".replace(",", " ")  # Форматируем целое число
        elif isinstance(value, float):
            value = f"{value:,.2f}".replace(",", " ")  # Форматируем число с плавающей точкой
        elif isinstance(value, str):
            try:
                # Пробуем преобразовать строку в число
                if "." in value:
                    numeric_value = float(value.replace(" ", "").replace(",", ""))
                    value = f"{numeric_value:,.2f}".replace(",", " ")
                else:
                    numeric_value = int(value.replace(" ", "").replace(",", ""))
                    value = f"{numeric_value:,}".replace(",", " ")
            except ValueError:
                pass

        for i in range(self.data_grid.count()):
            item = self.data_grid.itemAt(i)
            if item:
                tile_container = item.widget()
                if tile_container:
                    label = tile_container.findChild(QLabel, key)
                    if label:
                        value_label = tile_container.findChild(QLabel, f"{key}_value")
                        
                        if value_label:
                            if key == "Победы" and value != "-":
                                value_label.setText(value + "%")
                                try:
                                    win_percent = float(value.strip('%'))
                                    if win_percent >= 70.00:
                                        color = "#9989e6"
                                    elif 60.00 <= win_percent < 70.00:
                                        color = "#72d1ff"
                                    elif 50.00 <= win_percent < 60.00:
                                        color = "#a8e689"
                                    else:
                                        color = "#ffffff"
                                    value_label.setStyleSheet(f"""
                                        font-family: Consolas;
                                        font-size: 16px;
                                        font-weight: bold;
                                        color: {color};
                                        background-color: transparent;
                                    """)
                                except ValueError:
                                    pass  
                            elif key == "Выжил" and value != "-":
                                value_label.setText(value + "%")
                            elif key == "Точность" and value != "-":
                                value_label.setText(value + "%")
                            else:
                                value_label.setText(value)

    def mousePressEvent(self, event):
        if event.button() == Qt.RightButton:  # Логика для правой кнопки мыши
            self.show_context_menu(event.pos())
        # Передаём событие родительскому виджету
        super().mousePressEvent(event)

    def show_context_menu(self, pos):
        menu = QMenu(self)
        menu.setStyleSheet("""
            QMenu {
                background-color: rgb(57, 57, 57);
                color: #e2ded3;
                border: 1px solid #444;
            }
            QMenu::item:selected {      /* выделенный пункт */
                background-color: rgb(90, 90, 110);
                color: #ffffff;
            }
        """)
        for key in self.all_keys:  # Все плитки всегда
            action = QAction(key, self)
            action.setCheckable(True)
            action.setChecked(key in self.visible_tiles)
            action.toggled.connect(lambda checked, k=key: self.toggle_tile(k, checked))
            menu.addAction(action)

        action = QAction("Скрыть окно", self)
        action.setCheckable(True)
        action.setChecked(not self.config_visible)
        action.toggled.connect(lambda checked, k="Скрыть окно": self.toggle_visible(k, self.config_visible))
        menu.addAction(action)
        menu.exec_(self.mapToGlobal(pos))

    def toggle_tile(self, key, visible):
        if visible:
            self.visible_tiles.add(key)
        else:
            self.visible_tiles.discard(key)
        self.rebuild_grid()

    def toggle_visible(self, key, visible):
        if visible:
            self.stacked_widget.setStyleSheet("""
                QStackedWidget, QStackedWidget > QWidget {
                    background-color: rgba(30, 30, 30, 0);  /* Полупрозрачный фон */
                    border: none;
                    border-top-right-radius: 10px;
                    border-bottom-right-radius: 10px;
                }
            """)
            self.config_container.hide()
            self.config_visible = False
        else:
            self.stacked_widget.setStyleSheet("""
                QStackedWidget, QStackedWidget > QWidget {
                    background-color: rgba(30, 30, 30, 220);  /* Стандартный фон */
                    border: none;
                    border-top-right-radius: 10px;
                    border-bottom-right-radius: 10px;
                }
            """)
            self.config_container.show()
            self.config_visible = True
    
    def rebuild_grid(self):
        while self.data_grid.count():
            item = self.data_grid.takeAt(0)
            if item.widget():
                item.widget().deleteLater()

        self.tiles.clear()

        battles = self.api_client.main_stats_structure["battles"] + self.api_client.rating_stats_structure["battles"]
        if battles > 0:
            wins = self.api_client.main_stats_structure["wins"] + self.api_client.rating_stats_structure["wins"]
            total_damage = self.api_client.main_stats_structure["totalDamage"] + self.api_client.rating_stats_structure["totalDamage"]
            exp = self.api_client.main_stats_structure["exp_battle"] + self.api_client.rating_stats_structure["exp_battle"]
            master = self.api_client.master_structure["mastery"]

            if self.api_client.other_stats_structure["shots"] > 0:
                shots = round((self.api_client.other_stats_structure["hits"] / self.api_client.other_stats_structure["shots"]) * 100.00, 2)

            survived = round((self.api_client.other_stats_structure["survived"] / self.api_client.other_stats_structure["battles"]) * 100.00, 2)

            values = {
                "Бои": battles,
                "Победы": round((wins / battles) * 100, 2),
                "Урон": total_damage // battles,
                "Опыт": exp // battles,
                "Выжил": survived,
                "Точность": shots,
                "Мастер": master,
                "Рейтинг": "0"
            }

            rating_delta = self.api_client.rating_stats_structure["mm_rating"] - self.api_client.first_rating_stats_structure["mm_rating"]
            sign = "+" if rating_delta >= 0 else ""
            values["Рейтинг"] = f"{self.api_client.rating_stats_structure['mm_rating']} ({sign}{rating_delta})"
        else:
            values = {
                "Бои": battles,
                "Победы": "-",
                "Урон": "-",
                "Опыт": "-",
                "Выжил": "-",
                "Точность": "-",
                "Мастер": "-",
                "Рейтинг": "0"
            }
            rating_delta = self.api_client.rating_stats_structure["mm_rating"] - self.api_client.first_rating_stats_structure["mm_rating"]
            sign = "+" if rating_delta >= 0 else ""
            values["Рейтинг"] = f"{self.api_client.rating_stats_structure['mm_rating']} ({sign}{rating_delta})"

        visible_keys = [key for key in self.all_keys if key in self.visible_tiles]
        tiles_per_column = (len(visible_keys) + 1) // 2 if len(visible_keys) > 4 else len(visible_keys)

        for idx, key in enumerate(visible_keys):
            col = idx % 2 if len(visible_keys) > 4 else 0
            row = idx // 2 if len(visible_keys) > 4 else idx
            value = values.get(key, "-")
            self.add_label_and_value(self.data_grid, key, str(value), row, col)

class Info(QWidget):
    def __init__(self, api_client, main_stat, rating_stat, tank_stat, other_stat, stream_page):
        super().__init__()
        self.api_client = api_client
        self.main_stat = main_stat
        self.rating_stat = rating_stat
        self.tank_stat = tank_stat
        self.stream_page = stream_page
        self.other_stat = other_stat
        self.setup_ui()
    
    def setup_ui(self):
        main_layout = QVBoxLayout(self)
        main_layout.setSpacing(5)
        main_layout.setContentsMargins(10, 2, 10, 2)
        main_layout.setAlignment(Qt.AlignTop)

        title_layout = QHBoxLayout()
        title_layout.setSpacing(10)
        title_layout.setContentsMargins(10, 2, 10, 2)
        
        arrow_icon = QLabel()
        arrow_icon.setPixmap(QPixmap(resource_path('src/arrow_icon.webp')))
        arrow_icon.setStyleSheet("background-color: transparent;")
        title_layout.addWidget(arrow_icon)
        
        title = QLabel("STATS OVERLAY v1.0.1")
        title.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 14px;
            color: #e2ded3;
            background-color: transparent;
        """)
        title.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        title.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        title_layout.addWidget(title)
        main_layout.addLayout(title_layout)

        info_text = QLabel()
        info_text.setOpenExternalLinks(True)  # Разрешаем открытие внешних ссылок
        info_text.setTextFormat(Qt.RichText)  # Включаем HTML-форматирование
        info_text.setWordWrap(True)
        
        # HTML-текст с ссылками
        info_text.setText("""
        <p><b>Лицензионное соглашение</b></p>
                          
        <p>Используя приложение, вы соглашаетесь с лицензионным соглашением.</p>
        
        <p>Используется API Леста Игры:<br>
        - <a href="https://developers.lesta.ru/documentation/rules/agreement/" style="color: #72d1ff;">Условия использования API</a><br>
        - <a href="https://legal.lesta.ru/privacy-policy/" style="color: #72d1ff;">Политика конфиденциальности</a></p>

        <p>Благодарности:<br>
        - Участникам группы Blitz Hata VЫP за поддержку и идеи<br>
        - Леста Игры за предоставленный API<br>
        - Сообществу Tanks Blitz за тестирование</p>

        <p>Телеграмм-канал автора:<br>
        - <a href="https://t.me/tanksblitz_pmods" style="color: #72d1ff;">Telegram</a></p>

        <p>Запрещено:<br>
        - Коммерческое использование и распространение <br>
        - Модификация и декомпиляция кода <br>
        - Использование в нарушение правил Леста Игры <br>
        - Распространение приложения или его производных без указания автора</p>
        
        <p>Автор не гарантирует 100% точность данных и не несет ответственности за:<br>
        - Блокировки аккаунтов (используйте на свой риск)</p>                        
        """)
        
        info_text.setStyleSheet("""
            font-family: Times New Roman;
            font-size: 14px;
            color: #e2ded3;
            background-color: transparent;
        """)
        info_text.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        info_text.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        scroll_area = QScrollArea()
        scroll_area.setWidgetResizable(True)
        scroll_area.setWidget(info_text)
        scroll_area.setViewportMargins(5, 5, 5, 5)
        scroll_area.setStyleSheet("""
            QScrollArea {
                background-color: rgba(50, 50, 50, 80);
                border: none;
                border-radius: 5px;
            }
            QScrollBar:vertical {
                background: rgba(0, 0, 0, 0);
                width: 4px;
                margin: 2px 0;
                border-radius: 0px;
            }
            QScrollBar::handle:vertical {
                background: #555555;
                border-radius: 2px;
            }
            QScrollBar::groove:vertical {
                border-radius: 0px;
            }
            /* Горизонтальный скроллбар */
            QScrollBar:horizontal {
                background: rgba(0, 0, 0, 0);
                height: 4px;
                margin: 0 2px;
                border-radius: 0px;
            }
            QScrollBar::handle:horizontal {
                background: #555555;
                border-radius: 2px;
            }
            QScrollBar::groove:horizontal {
                border-radius: 0px;
            }
            /* Общие настройки */
            QScrollBar::add-line:vertical, 
            QScrollBar::sub-line:vertical,
            QScrollBar::add-line:horizontal, 
            QScrollBar::sub-line:horizontal {
                height: 0;
                width: 0;
            }
        """)
        scroll_area.setMaximumHeight(185)
        main_layout.addWidget(scroll_area)

        donate_button = QPushButton()
        donate_button.setText("Поддержать автора")
        donate_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(70, 70, 70, 150);
                color: #e2ded3;
                border: 1px solid #333333;
                font-size: 12px;
                font-family: Consolas;
                font-weight: bold;
                padding: 5px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: rgba(90, 90, 90, 200);
                color: #ffffff;
            }
            QPushButton:pressed {
                background-color: rgba(50, 50, 50, 200);
                color: #cccccc;
            }
        """)
        donate_button.clicked.connect(lambda: self.open_page_donate())  
        donate_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        main_layout.addWidget(donate_button)

        support_button = QPushButton()
        support_button.setText("Центр поддержки")
        support_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(70, 70, 70, 150);
                color: #e2ded3;
                border: 1px solid #333333;
                font-size: 12px;
                font-family: Consolas;
                font-weight: bold;
                padding: 5px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: rgba(90, 90, 90, 200);
                color: #ffffff;
            }
            QPushButton:pressed {
                background-color: rgba(50, 50, 50, 200);
                color: #cccccc;
            }
        """)
        support_button.clicked.connect(lambda: self.open_page_support())  
        support_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        main_layout.addWidget(support_button)

        reset_button = QPushButton()
        reset_button.setText("Сбросить статистику")
        reset_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(70, 70, 70, 150);
                color: #e2ded3;
                border: 1px solid #333333;
                font-size: 12px;
                font-family: Consolas;
                font-weight: bold;
                padding: 5px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: rgba(90, 90, 90, 200);
                color: #ffffff;
            }
            QPushButton:pressed {
                background-color: rgba(50, 50, 50, 200);
                color: #cccccc;
            }
        """)
        reset_button.clicked.connect(lambda: self.reset_data())  
        reset_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        main_layout.addWidget(reset_button)

        self.auth_button = QPushButton()
        self.auth_button.setStyleSheet("""
            QPushButton {
                background-color: rgba(70, 70, 70, 150);
                color: #e2ded3;
                border: 1px solid #333333;
                font-size: 12px;
                font-family: Consolas;
                font-weight: bold;
                padding: 5px;
                border-radius: 5px;
            }
            QPushButton:hover {
                background-color: rgba(90, 90, 90, 200);
                color: #ffffff;
            }
            QPushButton:pressed {
                background-color: rgba(50, 50, 50, 200);
                color: #cccccc;
            }
        """)

        if(self.api_client.is_auth):
            self.auth_button.setText("Выход")
        else:
            self.auth_button.setText("Авторизоваться")

        self.auth_button.clicked.connect(lambda: self.server_auth())  
        self.auth_button.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        main_layout.addWidget(self.auth_button)

        message_layout = QHBoxLayout()
        message_layout.setSpacing(10)
        message_layout.setContentsMargins(10, 2, 10, 2)
        
        self.message_text = QLabel("Пожалуйста, авторизуйтесь")
        self.message_text.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 14px;
            color: #99ff99;
            background-color: transparent;
        """)
        self.message_text.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        self.message_text.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)

        if(self.api_client.is_auth):
            self.message_text.setText(f"{self.api_client.nickname}")
        else:
            self.message_text.setText("Пожалуйста, авторизуйтесь")

        message_layout.addWidget(self.message_text)
        main_layout.addLayout(message_layout)

        lesta = QLabel("© Леста Игры. Все права защищены")
        lesta.setStyleSheet("""
            font-family: Segoe UI;
            font-weight: bold;
            font-size: 10px;
            color: #e2ded3;
            padding: 5px;
            background-color: transparent;
        """)
        lesta.setAlignment(Qt.AlignVCenter | Qt.AlignLeft)  
        lesta.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        main_layout.addWidget(lesta)

    def open_page_donate(self):
        donate_url = QUrl("https://www.donationalerts.com/r/parrylikeme")
        QDesktopServices.openUrl(donate_url)

    def open_page_support(self):
        donate_url = QUrl("https://lesta.ru/support/ru/")
        QDesktopServices.openUrl(donate_url)

    def server_auth(self):
        if self.api_client.is_auth:
            success = self.api_client.exit_auth()
            if success:
                self.reset_data()
                documents_path = os.path.expanduser("~/Documents/StatsOverlay")
                file_path = os.path.join(documents_path, "stats.json")
                try:
                    if os.path.exists(file_path):
                        os.remove(file_path)
                        print(f"Файл {file_path} удалён при выходе из аккаунта.")
                    else:
                        print(f"Файл {file_path} не найден при попытке удаления.")
                except Exception as e:
                    print(f"Ошибка при удалении файла: {e}")
                self.message_text.setText(f"Выполнен выход из аккаунта")
                self.auth_button.setText("Авторизация")
                self.is_auth = False
                self.api_client.is_auth = False
            else:
                self.message_text.setText("Ошибка выхода. Попробуйте снова.")
        else:
            def auth_thread():
                success = self.api_client.authenticate()
                if success:
                    while self.api_client.nickname is None:
                        time.sleep(0.1)

                    nickname = self.api_client.nickname
                    self.message_text.setText(f"{nickname}")
                    self.auth_button.setText("Выход")
                    self.is_auth = True
                    self.api_client.is_auth = True
                else:
                    self.message_text.setText("Ошибка авторизации. Попробуйте снова.")

            thread = threading.Thread(target=auth_thread)
            thread.daemon = True
            thread.start()

    def reset_data(self):
        self.main_stat.set_value("Золото", "-")
        self.main_stat.set_value("Кредиты", "-")
        self.main_stat.set_value("Боевой опыт", "-")
        self.main_stat.set_value("Cвободный опыт", "-")
        self.main_stat.set_value("Проведено боев", "-")
        self.main_stat.set_value("Победы", "-")
        self.main_stat.set_value("Урон", "-")
        self.main_stat.set_value("Опыт", "-")

        self.rating_stat.set_value("Текущий рейтинг", "-")
        self.rating_stat.set_value("Прогресс рейтинга", "-")
        self.rating_stat.set_value("Калибровочные бои", "-")
        self.rating_stat.set_value("Проведено боев", "-")
        self.rating_stat.set_value("Победы", "-")
        self.rating_stat.set_value("Урон", "-")
        self.rating_stat.set_value("Опыт", "-")

        self.other_stat.set_value("Процент попаданий", "-")
        self.other_stat.set_value("Процент выживания", "-")
        self.other_stat.set_value("Ср. время выживания", "-")
        self.other_stat.set_value("Коэффициент урона", "-")
        self.other_stat.set_value("Коэффициент уничтожения", "-")

        self.other_stat.set_value_masters("master_label", "-")
        self.other_stat.set_value_masters("master_label_1", "-")
        self.other_stat.set_value_masters("master_label_2", "-")
        self.other_stat.set_value_masters("master_label_3", "-")

        self.stream_page.set_value("Бои", "-")
        self.stream_page.set_value("Победы", "-")
        self.stream_page.set_value("Урон", "-")
        self.stream_page.set_value("Опыт", "-")
        self.stream_page.set_value("Выжил", "-")
        self.stream_page.set_value("Точность", "-")
        self.stream_page.set_value("Рейтинг", "-")
        self.stream_page.set_value("Мастер", "-")


        self.tank_stat.reset_tank_data.emit()
        self.api_client.save_current_to_first()

class ActivationWindow(QWidget):
    def __init__(self, on_activate_callback):
        super().__init__()
        self.on_activate_callback = on_activate_callback
        self.setWindowTitle("Активация")
        self.setWindowIcon(QIcon(resource_path('src/icon.ico')))
        self.setFixedSize(400, 250)
        self.setWindowFlags(Qt.WindowStaysOnTopHint | Qt.WindowCloseButtonHint)
        
        # Устанавливаем полупрозрачный темный фон
        self.setStyleSheet("""
            background-color: rgba(30, 30, 30, 153);
        """)
        
        self.setup_ui()
        
    def setup_ui(self):
        layout = QVBoxLayout()
        layout.setContentsMargins(20, 20, 20, 20)
        layout.setSpacing(15)
        layout.setSizeConstraint(QLayout.SetFixedSize)
        
        # Заголовок
        title = QLabel("Активация программы")
        title.setStyleSheet("""
            font-family: Segoe UI;
            font-size: 18px;
            font-weight: bold;
            color: #e2ded3;
            background-color: transparent;
        """)
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)
        
        # HWID информация
        hwid = HWIDActivator.get_hwid()
        self.hwid_label = QLabel(f"Ваш ID:\n{hwid}")
        self.hwid_label.setStyleSheet("""
            font-family: Consolas;
            font-size: 12px;
            color: #a8e689;
            background-color: rgba(42, 42, 42, 180);
            padding: 10px;
            border-radius: 5px;
        """)
        self.hwid_label.setAlignment(Qt.AlignCenter)
        self.hwid_label.setWordWrap(True)
        self.hwid_label.setFixedHeight(50)
        layout.addWidget(self.hwid_label)
        
        # Кнопка копирования
        copy_btn = QPushButton("Копировать ID")
        copy_btn.setStyleSheet("""
            QPushButton {
                background-color: rgba(56, 56, 56, 180);
                color: #e2ded3;
                border: 1px solid #555555;
                border-radius: 5px;
                padding: 5px;
            }
            QPushButton:hover {
                background-color: rgba(72, 72, 72, 180);
            }
        """)
        copy_btn.clicked.connect(self.copy_hwid)
        copy_btn.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Fixed)
        layout.addWidget(copy_btn)
        
        # Поле для ввода ключа
        self.key_input = QLineEdit()
        self.key_input.setPlaceholderText("Введите ключ активации...")
        self.key_input.setStyleSheet("""
            QLineEdit {
                background-color: rgba(42, 42, 42, 180);
                color: #e2ded3;
                border: 1px solid #555555;
                border-radius: 5px;
                padding: 8px;
            }
            QLineEdit[errorStyle="true"] {
                color: #ffaaaa;
            }
        """)
        layout.addWidget(self.key_input)
        
        # Кнопка активации
        activate_btn = QPushButton("Активировать")
        activate_btn.setStyleSheet("""
            QPushButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                border-radius: 5px;
                padding: 8px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #45a049;
            }
        """)
        activate_btn.clicked.connect(self.activate)
        layout.addWidget(activate_btn)
        
        self.setLayout(layout)
        
    def copy_hwid(self):
        clipboard = QApplication.clipboard()
        clipboard.setText(HWIDActivator.get_hwid())
        # Вместо MessageBox просто временно меняем текст кнопки
        sender = self.sender()
        original_text = sender.text()
        sender.setText("Скопировано!")
        QTimer.singleShot(2000, lambda: sender.setText(original_text))
        
    def activate(self):
        key = self.key_input.text().strip()
        
        if not key:
            # Пустое поле - меняем стиль и текст плейсхолдера
            self.key_input.setStyleSheet("""
                QLineEdit {
                    background-color: rgba(42, 42, 42, 180);
                    color: #ffaaaa;
                    border: 1px solid #555555;
                    border-radius: 5px;
                    padding: 8px;
                }
            """)
            self.key_input.setPlaceholderText("Пожалуйста, введите ключ...")
            return
            
        if HWIDActivator.activate(key):
            # Проверяем текущую дату
            current_date = datetime.now()
            expiration_date = datetime(2025, 8, 30, 23, 59, 59)  # Установленная дата и время

            if current_date > expiration_date:
                # Если текущая дата превышает лимит, блокируем доступ
                self.key_input.setText("")
                self.key_input.setStyleSheet("""
                    QLineEdit {
                        background-color: rgba(42, 42, 42, 180);
                        color: #ffaaaa;
                        border: 1px solid #555555;
                        border-radius: 5px;
                        padding: 8px;
                    }
                """)
                self.key_input.setPlaceholderText("Доступно обновление. Текущая версия недоступна.")
                return
            self.close()
            self.on_activate_callback(True)
        else:
            self.key_input.setText("")
            self.key_input.setStyleSheet("""
                QLineEdit {
                    background-color: rgba(42, 42, 42, 180);
                    color: #ffaaaa;
                    border: 1px solid #555555;
                    border-radius: 5px;
                    padding: 8px;
                }
            """)
            self.key_input.setPlaceholderText("Неверный ключ активации")

class SimplePlotWidget(FigureCanvas):
    def __init__(self, title: str, data: list[float], parent=None):
        self.fig = Figure(figsize=(4, 3), facecolor='none')  # прозрачный фон у всей фигуры
        super().__init__(self.fig)
        self.setParent(parent)

        self.ax = self.fig.add_subplot(111)
        self.ax.plot(data, marker='o', color='#e2ded3')
        self.ax.grid(True, color='gray', linestyle='--', alpha=0.3)

        x = list(range(len(data)))
        self.ax.fill_between(x, data, color='#e2ded3', alpha=0.2)

        for label in self.ax.get_xticklabels() + self.ax.get_yticklabels():
            label.set_fontname("Consolas")
            label.set_fontsize(9)
            label.set_color("#e2ded3")

        self.ax.set_facecolor('none')
        self.fig.patch.set_alpha(0.0)

        self.ax.tick_params(colors='#e2ded3')
        self.ax.spines['bottom'].set_color('#e2ded3')
        self.ax.spines['top'].set_color('#e2ded3')
        self.ax.spines['left'].set_color('#e2ded3')
        self.ax.spines['right'].set_color('#e2ded3')

        # Стиль Qt-виджета
        self.setStyleSheet("background: transparent;")
        self.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

        for spine in self.ax.spines.values():
            spine.set_visible(False)

        self.fig.subplots_adjust(left=0.12, right=1.00, top=1.00, bottom=0.12)

class MainApp:
    def __init__(self):
        self.app = QApplication(sys.argv)
        self.app.setStyle("Fusion")
        self.app.setWindowIcon(QIcon(resource_path('src/icon.ico')))

        self.api_client = APIClient(
            base_url="https://api.tanki.su/wot",
            application_id="4d9feb2c53c712fc6a87e026c990dd12"
        )

        # Сохраняем ссылки на оверлеи как атрибуты класса
        self.overlay1 = None
        self.overlay2 = None

        self.expiration_date = datetime(2025, 8, 30, 23, 59, 59)
        self.start_expiration_timer()
        
        # Проверяем активацию
        if not HWIDActivator.check_activation():
            self.show_activation_window()
        else:
            # Проверяем текущую дату
            current_date = datetime.now()
            expiration_date = datetime(2025, 8, 30, 23, 59, 59)  # Установленная дата и время

            if current_date > expiration_date:
                QApplication.quit()
            else:    
                self.show_main_app()

    def start_expiration_timer(self):
        """Запускает таймер для проверки истечения срока действия программы."""
        self.expiration_timer = QTimer()
        self.expiration_timer.timeout.connect(self.check_expiration_date)
        self.expiration_timer.start(10000)

    def check_expiration_date(self):
        """Проверяет текущую дату и завершает приложение, если срок действия истёк."""
        current_date = datetime.now()
        if current_date > self.expiration_date:
            print("Срок действия программы истёк. Завершаем работу.")
            QMessageBox.critical(None, "Обновление!", "Вышла новая версия программы. Приложение будет закрыто.")
            if(self.api_client.is_auth):
                self.api_client.exit_auth();
            QApplication.quit()

    def show_activation_window(self):
        self.activation_window = ActivationWindow(self.on_activation_result)
        self.activation_window.show()
        
    def show_main_app(self):
        # Если оверлеи уже созданы, просто показываем их
        if self.overlay1 is None:
            self.overlay2 = Overlay_info(api_client=self.api_client, offset_x=100)
            self.overlay1 = Overlay(with_icon=True, overlay_info=self.overlay2)
        
        self.overlay2.show()
        self.overlay1.show()
        
    def on_activation_result(self, success):
        if success:
            self.show_main_app()
        else:
            QApplication.quit()
            
    def run(self):
        sys.exit(self.app.exec_())


if __name__ == '__main__':
    main_app = MainApp()
    main_app.run()