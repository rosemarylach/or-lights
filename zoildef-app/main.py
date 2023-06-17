from kivy.config import Config
Config.set('kivy', 'keyboard_mode', 'dock')
from kivymd.app import MDApp
from kivy.lang import Builder
from kivy.core.window import Window

from kivy.app import App
from kivy.properties import ObjectProperty, NumericProperty, BooleanProperty
from kivy.graphics import Color
from kivy.uix.vkeyboard import VKeyboard
from kivy.graphics import Line
from kivy.uix.widget import Widget

from kivy.uix.screenmanager import ScreenManager, Screen

import socket
import threading
import time

from kivy.uix.image import Image
from kivy.clock import Clock
from kivy.graphics.texture import Texture
import cv2

import math
import sympy
from sympy import symbols, solve, lambdify
from scipy.optimize import fsolve

# Window.fullscreen = True
# Window.borderless = True
Window.left = 2000
Window.maximize()

class ZoilConfig():

    def __init__(self):
        # ZOIL Selection
        self.x = 0
        self.y = 0
        self.r = 0
        self.b = 0

        # Calibration params
        self.len_size = 38 # in
        self.width_size = 25 # in
        self.table_height = 98 # in
        self.h = 52

        self.default_h = 56
        self.default_l = 25
        self.default_w = 38

        self.pixel_l = 0
        self.pixel_w = 0

        # CONSTANTS FOR CALCULATIONS ----------------
        # self.STRUCT_HEIGHT = 
        self.X_OFFSET = 18 #in, offset from (0, 0) to center of panel
        self.Y_OFFSET = 18 #in, offset from (0, 0) to center of panel
        self.FOCUS_OFFSET = 0.217 # in offset from (0, 0) to focus true 0
        self.H_A = 1.76 #in, offset from 80-20 structure to pitch axis
        self.P = 0.46 + 7.56 #in, distance from center of pitch axis to face of side lights

        # useful dimensions of the light w/ respect to rotation points
        self.B_1 = 0.89 #in
        self.B_2 = 1.6 #in
        self.B_3 = 1.29 #in
        self.B_4 = 1.83 #in

        self.R_AXIS = 2 #in, distance from light rotation point to the center axis of focus mechanism

        ## NOT CORRECT, NEEDS TO BE UPDATED
        self.L_ARM = 3 #in, length of linkage (between the two rotation points)
        self.R_TOP = 1.83 #in, length between top linkage rotation point and center axis of focus mechanism

        self.KAPPA_0 = 3.42 #in, distance from light rotation point to bottom face of hexagon plate when focus mechanism is at d = 0

        self.LIGHT_DIAMETER = 1.96 #in
        self.LIGHT_ANGLE = 5 # degrees
        
        # END CALC CONSTANTS ------------------------

        # Network Information
        self.my_ip = socket.gethostbyname(socket.gethostname())
        self.cam_ip = "192.168.171.19"
        self.teaching_ips = ["192.168.171.32", "192.168.171.175"]
        self.port = 50000
        self.timeout = 20.0 # timeout to accept connection from arduino
        self.conn, self.conn_addr = None, None

        # regularly schedule transmit
        self.transmit_freq = 1 # Hz
        self.format_transmit()
        Clock.schedule_interval(self.auto_transmit, 1.0 / self.transmit_freq)

    def calc_pitch(self):
        # distance from panel's coenter to center of ZOIL, projected onto lit surface, in
        l_1 = math.sqrt((self.Y_OFFSET - self.y) ** 2 + (self.X_OFFSET - self.x) ** 2)
        l_2 = math.sqrt((self.Y_OFFSET - self.y) ** 2 + (self.X_OFFSET + self.x) ** 2)
        l_3 = math.sqrt((self.Y_OFFSET + self.y) ** 2 + (self.X_OFFSET - self.x) ** 2)
        l_4 = math.sqrt((self.Y_OFFSET + self.y) ** 2 + (self.X_OFFSET + self.x) ** 2)

        # distance from lit surface to pitch axis center, in
        h_c = self.h - self.H_A

        # pitch calculations, degrees
        gamma_1 = - math.degrees(math.atan(l_1 / h_c))
        gamma_2 = math.degrees(math.atan(l_2 / h_c))
        gamma_3 = math.degrees(math.atan(l_3 / h_c))
        gamma_4 = - math.degrees(math.atan(l_4 / h_c))

        return [gamma_1, gamma_2, gamma_3, gamma_4], [l_1, l_2, l_3, l_4]

    def calc_yaw(self):
        theta_1 = - math.degrees(math.atan((self.Y_OFFSET - self.y) / (self.X_OFFSET - self.x)))
        theta_2 = math.degrees(math.atan((self.Y_OFFSET - self.y) / (self.X_OFFSET + self.x)))
        theta_3 = math.degrees(math.atan((self.Y_OFFSET + self.y) / (self.X_OFFSET - self.x)))
        theta_4 = - math.degrees(math.atan((self.Y_OFFSET + self.y) / (self.X_OFFSET + self.x)))

        return [theta_1, theta_2, theta_3, theta_4]
    
    def calc_focus(self, pitch, l_dists):
        # declare symbolic variables for alphas
        alpha_syms = [ symbols('alpha_1_sym'),
                       symbols('alpha_2_sym'),
                       symbols('alpha_3_sym'),
                       symbols('alpha_4_sym') ]

        # offset from pitch axis to side light height, in
        h_bs = [self.P * math.cos(math.radians(gamma)) for gamma in pitch]

        # average distance that light will travel to reach lit surface, in
        lambdas = [math.sqrt(l ** 2 + (self.h - self.H_A - h_b) ** 2) for l, h_b in zip(l_dists, h_bs)]

        # diameter of projected light from side lights, in
        dsls = [self.LIGHT_DIAMETER + 2 * lam * math.tan(math.radians(self.LIGHT_ANGLE)) for lam in lambdas]

        # distance from center of ZOIL to edge of projected light by side light (circumradius of "hexagon"), in
        a = 2 / math.sqrt(3) * self.r

        # distance from center of ZOIL to center of projected light by side light, in
        qs = [a - 1/2 * dsl for dsl in dsls]

        # change in distance from light center to panel focus mechanism center due to tilt of light
        # SYMBOLIC
        delta_rs = [self.B_1 * sympy.cos(alpha) - self.B_2 * sympy.sin(alpha) for alpha in alpha_syms]

        # distance between center of light and panel focus mechanism center
        # SYMBOLIC
        r_bcs = [self.R_AXIS + delta_r for delta_r in delta_rs]

        # distance from center of projected light by side light to center of side light projected downwards onto lit surface, in
        # SYMBOLIC
        ms = [r_bc - q for r_bc, q in zip(r_bcs, qs)]

        # expression for each of the alphas
        eqs = [sympy.atan(m / lam) - alpha for m, lam, alpha in zip(ms, lambdas, alpha_syms)]

        # convert to numpy for solving numerically instead of analytically
        funcs = [lambdify(alpha, eq, modules=['numpy']) for alpha, eq in zip(alpha_syms, eqs)]

        # solve the expressions, radians
        solved_alphas = [fsolve(func, 0) for func in funcs]

        # distance from base of linkage to center axis of focus mechanism, in
        ws = [self.R_AXIS + self.B_3 * math.cos(alpha) + self.B_4 * math.sin(alpha) for alpha in solved_alphas]

        # height between light rotation point and base of hexagon plate
        kappas = [math.sqrt(self.L_ARM ** 2 - (w - self.R_TOP) ** 2) + self.B_4 * math.cos(alpha) - self.B_3 * math.sin(alpha) for w, alpha in zip(ws, solved_alphas)]

        # target distance for focus mechanism to be at
        ds = [kappa - self.KAPPA_0 - self.FOCUS_OFFSET for kappa in kappas]

        return ds
    
    def calc_all(self):
        pitch, l_dists = self.calc_pitch()
        yaw = self.calc_yaw()
        focus = self.calc_focus(pitch, l_dists)

        return pitch, yaw, focus

    def connect(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.settimeout(self.timeout)
        s.bind((self.my_ip, self.port))
        s.listen(1)
        self.conn, self.conn_addr = s.accept()

    def update_zoil(self, x, y, r, b):
        self.x = x
        self.y = y
        self.r = r
        self.b = b

    def format_transmit(self):
        if self.r == 0:
            pitch, yaw, focus = [0, 0, 0, 0], [0, 0, 0, 0], [0, 0, 0, 0]
        else:
            pitch, yaw, focus = self.calc_all()

        self.send_str = "f1:{:.2f}p1:{:.2f}y1:{:.2f}" \
        "f2:{:.2f}p2:{:.2f}y2:{:.2f}" \
        "f3:{:.2f}p3:{:.2f}y3:{:.2f}" \
        "f4:{:.2f}p4:{:.2f}y4:{:.2f}" \
        "b:{}\n".format(
                focus[0], pitch[0], yaw[0],
                focus[1], pitch[1], yaw[1],
                focus[2], pitch[2], yaw[2],
                focus[3], pitch[3], yaw[3],
                self.b
                )

        # print(self.send_str)

    def auto_transmit(self, dt):
        if self.conn is not None:
            self.conn.sendall(bytes(self.send_str, 'utf-8'))

    def force_transmit(self, send_str):
        if self.conn is not None:
            self.conn.sendall(bytes(send_str, 'utf-8'))

# Define our different screens
class LandingPage(Screen):
    pass

class HomePage(Screen):
    pass

class CalibrationPage(Screen):
    pass

class DefinePage(Screen):
    pass

class TeachingPage(Screen):
    pass

class PageManager(ScreenManager):
    pass

class KivyCamera(Image):

    capture = ObjectProperty(None)
    fps = NumericProperty(10)
    crop_scale = NumericProperty(0.45)
    rot = BooleanProperty(True)
    flip = BooleanProperty(True)
    to_update = BooleanProperty(True)

    def __init__(self, **kwargs):
        super(KivyCamera, self).__init__(**kwargs)

        Clock.schedule_interval(self.update, 1.0 / self.fps)

    def update(self, dt):
        if self.capture is not None and self.to_update is True:
            # ret, frame = self.capture.read()
            ret, frame = self.capture.ret, self.capture.frame
            if ret and frame is not None:
                # convert it to texture
                rot_flip = frame
                if self.flip:
                    rot_flip = cv2.flip(frame, 0)
                if self.rot:
                    rot_flip = cv2.rotate(rot_flip, cv2.ROTATE_90_CLOCKWISE)

                crop = rot_flip[int(rot_flip.shape[0]/2 - rot_flip.shape[0]*self.crop_scale/2):int(rot_flip.shape[0]/2 + rot_flip.shape[0]*self.crop_scale/2),
                    int(rot_flip.shape[1]/2 - rot_flip.shape[1]*self.crop_scale/2):int(rot_flip.shape[1]/2 + rot_flip.shape[1]*self.crop_scale/2)]
                buf = crop.tobytes()
                # flip crop dimensions in size because orgiginal frame was rotated
                image_texture = Texture.create(
                    size=(crop.shape[1], crop.shape[0]), colorfmt='bgr')
                image_texture.blit_buffer(buf, colorfmt='bgr', bufferfmt='ubyte')
                # display image from the texture
                self.texture = image_texture

            else:
                self.source = "images/body.jpg"
    
        else:
            self.source = "images/body.jpg"

class CircleDrawingWidget(Widget):

    def __init__(self, **kwargs):
        super(CircleDrawingWidget, self).__init__(**kwargs)
        self.cent_x = 0
        self.cent_y = 0
        self.rad = 0

        self.prev_x = 0
        self.prev_y = 0
        self.prev_rad = 0

        # self.min_rad = 0
        # self.max_rad = self.width
        with self.canvas:
            self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad), width=2.5)

    def draw_circle(self):
        with self.canvas:
            self.canvas.remove(self.circ)
            Color(1, 0, 1, 1, mode="rgba")
            self.circ = Line(circle=(self.cent_x, self.cent_y, self.rad), width=2.5)

    def shift(self, x_shift, y_shift):
        new_cent_x = self.cent_x + x_shift
        new_cent_y = self.cent_y + y_shift

        if (self.collide_point(new_cent_x + self.rad, new_cent_y) and
            self.collide_point(new_cent_x - self.rad, new_cent_y) and
            self.collide_point(new_cent_x, new_cent_y + self.rad) and
            self.collide_point(new_cent_x, new_cent_y - self.rad)):

            self.cent_x = new_cent_x
            self.cent_y = new_cent_y

            self.draw_circle()
        
    def resize(self, rad_change):
        new_rad = self.rad + rad_change

        if (self.collide_point(self.cent_x + new_rad, self.cent_y) and
            self.collide_point(self.cent_x - new_rad, self.cent_y) and
            self.collide_point(self.cent_x, self.cent_y + new_rad) and
            self.collide_point(self.cent_x, self.cent_y - new_rad) and
            new_rad >= 0):

            self.rad = new_rad

            self.draw_circle()

    def on_touch_down(self, touch):
        if self.collide_point(touch.x, touch.y):
            self.cent_x = touch.x
            self.cent_y = touch.y
            self.rad = 0
            
            self.draw_circle()

    def on_touch_move(self, touch):
        
        new_rad = math.sqrt((self.cent_x - touch.x)**2 + (self.cent_y - touch.y)**2)

        if (self.collide_point(self.cent_x + new_rad, self.cent_y) and
            self.collide_point(self.cent_x - new_rad, self.cent_y) and
            self.collide_point(self.cent_x, self.cent_y + new_rad) and
            self.collide_point(self.cent_x, self.cent_y - new_rad)):
            
            self.rad = new_rad

            self.draw_circle()

    # def on_touch_up(self, touch):
    #     if self.collide_point(touch.x, touch.y):
    #         self.len_size = 6.75
    #         self.width_size = 2

    #         diff_x = (self.cent_x - self.center_x) * self.width_size / self.width
    #         diff_y = (self.cent_y - self.center_y) * self.len_size / self.height
    #         diff_rad = self.rad * self.width_size / self.width
    #         # print(self.ids.vid.norm_image_size)
    #         # print("full center: ", self.ids.vid.center_x, self.ids.vid.center_y)
    #         # print("circ center: ", self.cent_x, self.cent_y, self.rad)
    #         print("coords: ", diff_x, diff_y, diff_rad)

    def reset(self):
        self.cent_x = 0
        self.cent_y = 0
        self.rad = 0

        self.draw_circle()

class RectDrawingWidget(Widget):
    def __init__(self, **kwargs):
        super(RectDrawingWidget, self).__init__(**kwargs)

        self.rect_x = 0
        self.rect_y = 0
        self.rect_w = 0
        self.rect_l = 0

        self.prev_x = 0
        self.prev_y = 0
        self.prev_w = 0
        self.prev_l = 0

        self.corner1 = (0, 0)
        self.corner2 = (0, 0)

        with self.canvas:
            self.rect = Line(rectangle=(0, 0, 0, 0), width=2.5)

    def draw_rect(self, x, y, w, l):
        with self.canvas:
            self.canvas.remove(self.rect)
            Color(1, 0, 1, 1, mode="rgba")
            self.rect = Line(rectangle=(x, y, w, l), width=2.5)

    def on_touch_down(self, touch):
        self.corner1 = (touch.x, touch.y)
        self.corner2 = (touch.x, touch.y)

        if self.collide_point(touch.x, touch.y):
            self.draw_rect(touch.x, touch.y, 0, 0)

    def on_touch_move(self, touch):
        self.corner2 = (touch.x, touch.y)

        if (self.collide_point(self.corner1[0], self.corner1[1]) and
            self.collide_point(self.corner2[0], self.corner2[1])):

            self.rect_x = min(self.corner1[0], self.corner2[0])
            self.rect_y = min(self.corner1[1], self.corner2[1])

            self.rect_l = abs(self.corner1[1] - self.corner2[1])
            self.rect_w = abs(self.corner1[0] - self.corner2[0])

            self.draw_rect(self.rect_x, self.rect_y, self.rect_w, self.rect_l)

    def reset(self):
        self.rect_x = 0
        self.rect_y = 0
        self.rect_w = 0
        self.rect_l = 0

        self.draw_rect(0, 0, 0, 0)

    def is_empty(self):
        return (self.rect_x == 0 and
                self.rect_y == 0 and
                self.rect_w == 0 and
                self.rect_l == 0)
    
    def set_inputs(self, config, length_input, width_input, height_input):
        if length_input.text == '':
            config.len_size = config.default_l
        else:
            try:
                config.len_size = float(length_input.text)
            except ValueError:
                length_input.text = ''

        if width_input.text == '':
            config.width_size = config.default_w
        else:
            try:
                config.width_size = float(width_input.text)
            except ValueError:
                width_input.text = ''

        if height_input.text == '':
            config.h = config.default_h
        else:
            try:
                config.h = config.table_height - float(height_input.text)
            except ValueError:
                height_input.text = ''

    # def on_touch_up(self, touch):
    #     if self.collide_point(touch.x, touch.y):
    #         self.len_size = 6.75
    #         self.width_size = 2

    #         pos_x = min(self.corner1[0], self.corner2[0])
    #         pos_y = min(self.corner1[1], self.corner2[1])

    #         cal_height = abs(self.corner1[1] - self.corner2[1])
    #         cal_width = abs(self.corner1[0] - self.corner2[0])

    #         # diff_x = (self.cent_x - self.center_x) * self.width_size / self.width
    #         # diff_y = (self.cent_y - self.center_y) * self.len_size / self.height
    #         # diff_rad = self.rad * self.width_size / self.width
    #         # print(self.ids.vid.norm_image_size)
    #         # print("full center: ", self.ids.vid.center_x, self.ids.vid.center_y)
    #         # print("circ center: ", self.cent_x, self.cent_y, self.rad)
    #         print("coords: ", pos_x, pos_y, cal_height, cal_width)

class CameraBufferCleanerThread(threading.Thread):
    def __init__(self, camera, name='camera-buffer-cleaner-thread'):
        self.camera = camera
        self.ret = None
        self.frame = None
        self.close = False
        super(CameraBufferCleanerThread, self).__init__(name=name)
        self.start()

    def run(self):
        while True:
            self.ret, self.frame = self.camera.read()

            if (self.close):
                return

class ZoilApp(MDApp):
    title = "ZoilDef App"

    my_config = ZoilConfig()

    zoil_def_ip = "rtsp://zoilcam:zoilZOIL!@" + my_config.cam_ip + "/stream1"
    zoil_def_capture = CameraBufferCleanerThread(cv2.VideoCapture(zoil_def_ip), name="zoil_thread")
    # zoil_def_capture.set(cv2.CAP_PROP_BUFFERSIZE, 2)

    teaching1_ip = "rtsp://zoilcam:zoilZOIL!@" + my_config.teaching_ips[0] + "/stream1"
    teaching1_capture = CameraBufferCleanerThread(cv2.VideoCapture(teaching1_ip), name="teaching1_thread")
    # teaching1_capture.set(cv2.CAP_PROP_BUFFERSIZE, 2)

    teaching2_ip = "rtsp://zoilcam:zoilZOIL!@" + my_config.teaching_ips[1] + "/stream1"
    teaching2_capture = CameraBufferCleanerThread(cv2.VideoCapture(teaching2_ip), name="teaching2_thread")
    # teaching2_capture.set(cv2.CAP_PROP_BUFFERSIZE, 2)

    # keyboard = Window.request_keyboard(self._keyboard_close, self)
    
    # if keyboard.widget:
    #     keyboard.widget.layout = 'numeric.json'

    # def _keyboard_close(self):
    #     pass

    def build(self):
        # layout =  ZoilDefLayout()
        layout = Builder.load_file("zoildef.kv")
        return layout

    def on_pause(self):
        ZoilApp.zoil_def_capture.close = True
        ZoilApp.teaching1_capture.close = True
        ZoilApp.teaching2_capture.close = True
        return True
    
    def on_stop(self):
        ZoilApp.zoil_def_capture.close = True
        ZoilApp.teaching1_capture.close = True
        ZoilApp.teaching2_capture.close = True
        return True


if __name__ == '__main__':
    Window.clearcolor = (0, 0, 0, 1)
    ZoilApp().run()