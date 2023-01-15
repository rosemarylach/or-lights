from kivy.app import App
from kivy.lang import Builder
from kivy.core.window import Window
from kivy.app import App
from kivy.graphics import Rectangle
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
    # def __init__(self, **kwargs):
    #     super(Touch, self).__init__(**kwargs)

    #     with self.canvas:
    #         Line(points=(20,30,400,500,60,500))
    #         Color(1,0,0,0.5, mode="rgba")
    #         self.rect = Rectangle(pos=(0,0), size=(50,50))

    # def on_touch_down(self, touch):
    #     with self.canvas:
    #         Color(1, 0, 0, 0.5, mode="rgba")
    #         self.x = touch.pos[0]
    #         self.y = touch.pos[1]
    #         self.line = Line(circle=(self.x, self.y, 1))

    # def on_touch_move(self, touch):
    #     with self.canvas:
    #         Color(1, 0, 0, 0.5, mode="rgba")
    #         self.line = Line(circle=(self.x, self.y, math.sqrt((self.x - touch.pos[0])**2 + (self.y - touch.pos[1])**2)))
    
    def on_touch_down(self, touch):
        with self.canvas:
            self.cent_x = touch.pos[0]
            self.cent_y = touch.pos[1]
            self.circ = Line(circle=(self.cent_x, self.cent_y, 0))

    def on_touch_move(self, touch):
        with self.canvas:
            self.canvas.remove(self.circ)
            Color(1, 0, 0, 0.5, mode="rgba")
            self.rad = math.sqrt((self.cent_x - touch.pos[0])**2 + (self.cent_y - touch.pos[1])**2)
            self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

    def on_touch_up(self, touch):
        self.len_size = 6.75
        self.width_size = 2

        diff_x = (self.cent_x - self.ids.vid.center_x) * self.width_size / self.ids.vid.norm_image_size[0]
        diff_y = (self.cent_y - self.ids.vid.center_y) * self.len_size / self.ids.vid.norm_image_size[1]
        diff_rad = self.rad * self.width_size / self.ids.vid.norm_image_size[0]
        # print(self.ids.vid.norm_image_size)
        # print("full center: ", self.ids.vid.center_x, self.ids.vid.center_y)
        # print("circ center: ", self.cent_x, self.cent_y, self.rad)
        print("coords: ", diff_x, diff_y, diff_rad)

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
    def __init__(self, fps=50, **kwargs):
        super(KivyCamera, self).__init__(**kwargs)
        ip = "rtsp://zoilcam:zoilZOIL!@10.216.77.47/stream1"
        capture = cv2.VideoCapture(ip)
        self.capture = capture
        Clock.schedule_interval(self.update, 1.0 / fps)

    def update(self, dt):
        ret, frame = self.capture.read()
        if ret:
            # convert it to texture
            rot_flip = cv2.rotate(cv2.flip(frame, 0), cv2.ROTATE_90_CLOCKWISE)
            crop_dim_scaling = 1/3
            crop = rot_flip[int(rot_flip.shape[0]/2 - rot_flip.shape[0]*crop_dim_scaling/2):int(rot_flip.shape[0]/2 + rot_flip.shape[0]*crop_dim_scaling/2),
                int(rot_flip.shape[1]/2 - rot_flip.shape[1]*crop_dim_scaling/2):int(rot_flip.shape[1]/2 + rot_flip.shape[1]*crop_dim_scaling/2)]
            buf = crop.tostring()

            # flip crop dimensions in size because orgiginal frame was rotated
            image_texture = Texture.create(
                size=(crop.shape[1], crop.shape[0]), colorfmt='bgr')
            image_texture.blit_buffer(buf, colorfmt='bgr', bufferfmt='ubyte')
            # display image from the texture
            self.texture = image_texture

# class DrawingWidget(Widget):
#     def __init__(self, **kwargs):
#         super(DrawingWidget, self).__init__(**kwargs)

#     def on_touch_down(self, touch):
#         with self.canvas:
#             self.cent_x = touch.pos[0]
#             self.cent_y = touch.pos[1]
#             self.circ = Line(circle=(self.cent_x, self.cent_y, 0))

#     def on_touch_move(self, touch):
#         with self.canvas:
#             self.canvas.clear()
#             Color(1, 0, 0, 0.5, mode="rgba")
#             self.rad = math.sqrt((self.cent_x - touch.pos[0])**2 + (self.cent_y - touch.pos[1])**2)
#             self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad))

#     def on_touch_up(self, touch):
#         self.len_size = 2
#         self.width_size = 4
#         print(self.ids.vid.norm_image_size)
#         # print(self.ids)
#         print("full center: ", self.center_x, self.center_y)
#         print("circ center: ", self.cent_x, self.cent_y, self.rad)


layout = Builder.load_file("zoildef.kv")

class ZoilDefApp(App):
    title = "ZoilDef App"

    def build(self):
        # layout =  ZoilDefLayout()
        return layout

    def on_pause(self):
        return True


if __name__ == '__main__':
    ZoilDefApp().run()