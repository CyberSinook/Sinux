#pragma once
void pic_init(void);
void pic_eoi(int irq);
void pic_mask(int irq);
void pic_unmask(int irq);
