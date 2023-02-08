from kivy.app import App
from kivymd.app import MDApp
from kivymd.uix.slider import slider
from kivymd.uix.button import button
from kivy.lang import Builder
from kivy.core.window import Window
from kivy.app import App
from kivy.graphics import Color

from kivy.graphics import Line
from kivy.uix.widget import Widget

from kivy.uix.floatlayout import FloatLayout
from kivy.uix.boxlayout import BoxLayout
from kivy.uix.relativelayout import RelativeLayout
from kivy.uix.screenmanager import ScreenManager, Screen
import time

from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture
import math
import cv2

# Define our different screens
class LandingPage(Screen):
    pass

class HomePage(Screen):
    pass

class CalibrationPage(Screen):
    pass

class DefinePage(Screen):
    pass

class AdjustPage(Screen):
    pass

class PageManager(ScreenManager):
    pass

# root widget for .kv file
# class ZoilDefLayout(FloatLayout):
#     def capture(self):
#         '''
#         Function to capture the images and give them the names
#         according to their captured time and date.
#         '''
#         camera = self.ids['camera']
#         timestr = time.strftime("%Y%m%d_%H%M%S")
#         camera.export_to_png("IMG_{}.png".format(timestr))
#         print("Captured")

class KivyCamera(Image):
    ip = "rtsp://zoilcam:zoilZOIL!@168.5.71.3/stream1"
    capture = cv2.VideoCapture(ip)

    def __init__(self, fps=10, **kwargs):
        super(KivyCamera, self).__init__(**kwargs)
        # self.source = "images/body.jpg"
        Clock.schedule_interval(self.update, 1.0 / fps)

    def update(self, dt):
        ret, frame = KivyCamera.capture.read()
        if ret:
            # convert it to texture
            rot_flip = cv2.rotate(cv2.flip(frame, 0), cv2.ROTATE_90_CLOCKWISE)
            crop_dim_scaling = 1/3
            crop = rot_flip[int(rot_flip.shape[0]/2 - rot_flip.shape[0]*crop_dim_scaling/2):int(rot_flip.shape[0]/2 + rot_flip.shape[0]*crop_dim_scaling/2),
                int(rot_flip.shape[1]/2 - rot_flip.shape[1]*crop_dim_scaling/2):int(rot_flip.shape[1]/2 + rot_flip.shape[1]*crop_dim_scaling/2)]
            buf = crop.tobytes()

            # flip crop dimensions in size because orgiginal frame was rotated
            image_texture = Texture.create(
                size=(crop.shape[1], crop.shape[0]), colorfmt='bgr')
            image_texture.blit_buffer(buf, colorfmt='bgr', bufferfmt='ubyte')
            # display image from the texture
            self.texture = image_texture

class CircleDrawingWidget(Widget):

    def __init__(self, **kwargs):
        super(CircleDrawingWidget, self).__init__(**kwargs)
        self.cent_x = 0
        self.cent_y = 0
        self.rad = 0

        # self.min_rad = 0
        # self.max_rad = self.width
        with self.canvas:
            self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def shift(self, x_shift, y_shift):
        self.cent_x -= x_shift
        self.cent_y -= y_shift
        
    def resize(self, new_rad):
        self.rad = new_rad

    def on_touch_down(self, touch):
        if self.collide_point(touch.x, touch.y):
            self.cent_x = touch.x
            self.cent_y = touch.y
            self.rad = 0
            with self.canvas:
                self.canvas.remove(self.circ)
                self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def on_touch_move(self, touch):
        
        new_rad = math.sqrt((self.cent_x - touch.x)**2 + (self.cent_y - touch.y)**2)

        if (self.collide_point(self.cent_x + new_rad, self.cent_y) and
            self.collide_point(self.cent_x - new_rad, self.cent_y) and
            self.collide_point(self.cent_x, self.cent_y + new_rad) and
            self.collide_point(self.cent_x, self.cent_y - new_rad)):
            
            self.rad = new_rad

            with self.canvas:
                self.canvas.remove(self.circ)
                Color(1, 0, 0, 1, mode="rgba")
                self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def on_touch_up(self, touch):
        if self.collide_point(touch.x, touch.y):
            self.len_size = 6.75
            self.width_size = 2

            diff_x = (self.cent_x - self.center_x) * self.width_size / self.width
            diff_y = (self.cent_y - self.center_y) * self.len_size / self.height
            diff_rad = self.rad * self.width_size / self.width
            # print(self.ids.vid.norm_image_size)
            # print("full center: ", self.ids.vid.center_x, self.ids.vid.center_y)
            # print("circ center: ", self.cent_x, self.cent_y, self.rad)
            print("coords: ", diff_x, diff_y, diff_rad)

class RectDrawingWidget(Widget):
    def __init__(self, **kwargs):
        super(RectDrawingWidget, self).__init__(**kwargs)
        with self.canvas:
            self.rect = Line(rectangle=(0, 0, 0, 0))

    def on_touch_down(self, touch):
        self.corner1 = (touch.x, touch.y)
        self.corner2 = (touch.x, touch.y)

        if self.collide_point(touch.x, touch.y):
            with self.canvas:
                self.canvas.remove(self.rect)
                self.rect = Line(rectangle=(touch.x, touch.y, 0, 0))

    def on_touch_move(self, touch):
        self.corner2 = (touch.x, touch.y)

        if (self.collide_point(self.corner1[0], self.corner1[1]) and
            self.collide_point(self.corner2[0], self.corner2[1])):

            pos_x = min(self.corner1[0], self.corner2[0])
            pos_y = min(self.corner1[1], self.corner2[1])

            cal_height = abs(self.corner1[1] - self.corner2[1])
            cal_width = abs(self.corner1[0] - self.corner2[0])

            with self.canvas:
                self.canvas.remove(self.rect)
                Color(0, 1, 0, 1, mode="rgba")
                self.rect = Line(rectangle=(pos_x, pos_y, cal_width, cal_height))

    def on_touch_up(self, touch):
        if self.collide_point(touch.x, touch.y):
            self.len_size = 6.75
            self.width_size = 2

            pos_x = min(self.corner1[0], self.corner2[0])
            pos_y = min(self.corner1[1], self.corner2[1])

            cal_height = abs(self.corner1[1] - self.corner2[1])
            cal_width = abs(self.corner1[0] - self.corner2[0])

            # diff_x = (self.cent_x - self.center_x) * self.width_size / self.width
            # diff_y = (self.cent_y - self.center_y) * self.len_size / self.height
            # diff_rad = self.rad * self.width_size / self.width
            # print(self.ids.vid.norm_image_size)
            # print("full center: ", self.ids.vid.center_x, self.ids.vid.center_y)
            # print("circ center: ", self.cent_x, self.cent_y, self.rad)
            print("coords: ", pos_x, pos_y, cal_height, cal_width)


class ZoilApp(MDApp):
    title = "ZoilDef App"

    def build(self):
        # layout =  ZoilDefLayout()
        layout = Builder.load_file("zoildef.kv")
        return layout

    def on_pause(self):
        return True


if __name__ == '__main__':
    Window.clearcolor = (0, 0, 0, 1)
    ZoilApp().run()