#include "ir_pch.h"

#include "Atmosphere.h"
#include <stb_image_write.h>
#include <glad\glad.h>


#define GL_RGBA32F 0x8814
#define GL_RGB32F 0x8815
#define GL_RGBA16F 0x881A
#define GL_RGB16F 0x881B
#define GL_RGBA 0x1908


namespace iara {
		constexpr int TRANSMITTANCE_TEXTURE_WIDTH = 256;
		constexpr int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

		constexpr int SCATTERING_TEXTURE_R_SIZE = 32;
		constexpr int SCATTERING_TEXTURE_MU_SIZE = 128;
		constexpr int SCATTERING_TEXTURE_MU_S_SIZE = 32;
		constexpr int SCATTERING_TEXTURE_NU_SIZE = 8;

		constexpr int SCATTERING_TEXTURE_WIDTH =
			SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
		constexpr int SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
		constexpr int SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;

		constexpr int IRRADIANCE_TEXTURE_WIDTH = 64;
		constexpr int IRRADIANCE_TEXTURE_HEIGHT = 16;

		// The conversion factor between watts and lumens.
		constexpr double MAX_LUMINOUS_EFFICACY = 683.0;

		// Values from "CIE (1931) 2-deg color matching functions", see
		// "http://web.archive.org/web/20081228084047/
		//    http://www.cvrl.org/database/data/cmfs/ciexyz31.txt".
		constexpr double CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[380] = {
		  360, 0.000129900000, 0.000003917000, 0.000606100000,
		  365, 0.000232100000, 0.000006965000, 0.001086000000,
		  370, 0.000414900000, 0.000012390000, 0.001946000000,
		  375, 0.000741600000, 0.000022020000, 0.003486000000,
		  380, 0.001368000000, 0.000039000000, 0.006450001000,
		  385, 0.002236000000, 0.000064000000, 0.010549990000,
		  390, 0.004243000000, 0.000120000000, 0.020050010000,
		  395, 0.007650000000, 0.000217000000, 0.036210000000,
		  400, 0.014310000000, 0.000396000000, 0.067850010000,
		  405, 0.023190000000, 0.000640000000, 0.110200000000,
		  410, 0.043510000000, 0.001210000000, 0.207400000000,
		  415, 0.077630000000, 0.002180000000, 0.371300000000,
		  420, 0.134380000000, 0.004000000000, 0.645600000000,
		  425, 0.214770000000, 0.007300000000, 1.039050100000,
		  430, 0.283900000000, 0.011600000000, 1.385600000000,
		  435, 0.328500000000, 0.016840000000, 1.622960000000,
		  440, 0.348280000000, 0.023000000000, 1.747060000000,
		  445, 0.348060000000, 0.029800000000, 1.782600000000,
		  450, 0.336200000000, 0.038000000000, 1.772110000000,
		  455, 0.318700000000, 0.048000000000, 1.744100000000,
		  460, 0.290800000000, 0.060000000000, 1.669200000000,
		  465, 0.251100000000, 0.073900000000, 1.528100000000,
		  470, 0.195360000000, 0.090980000000, 1.287640000000,
		  475, 0.142100000000, 0.112600000000, 1.041900000000,
		  480, 0.095640000000, 0.139020000000, 0.812950100000,
		  485, 0.057950010000, 0.169300000000, 0.616200000000,
		  490, 0.032010000000, 0.208020000000, 0.465180000000,
		  495, 0.014700000000, 0.258600000000, 0.353300000000,
		  500, 0.004900000000, 0.323000000000, 0.272000000000,
		  505, 0.002400000000, 0.407300000000, 0.212300000000,
		  510, 0.009300000000, 0.503000000000, 0.158200000000,
		  515, 0.029100000000, 0.608200000000, 0.111700000000,
		  520, 0.063270000000, 0.710000000000, 0.078249990000,
		  525, 0.109600000000, 0.793200000000, 0.057250010000,
		  530, 0.165500000000, 0.862000000000, 0.042160000000,
		  535, 0.225749900000, 0.914850100000, 0.029840000000,
		  540, 0.290400000000, 0.954000000000, 0.020300000000,
		  545, 0.359700000000, 0.980300000000, 0.013400000000,
		  550, 0.433449900000, 0.994950100000, 0.008749999000,
		  555, 0.512050100000, 1.000000000000, 0.005749999000,
		  560, 0.594500000000, 0.995000000000, 0.003900000000,
		  565, 0.678400000000, 0.978600000000, 0.002749999000,
		  570, 0.762100000000, 0.952000000000, 0.002100000000,
		  575, 0.842500000000, 0.915400000000, 0.001800000000,
		  580, 0.916300000000, 0.870000000000, 0.001650001000,
		  585, 0.978600000000, 0.816300000000, 0.001400000000,
		  590, 1.026300000000, 0.757000000000, 0.001100000000,
		  595, 1.056700000000, 0.694900000000, 0.001000000000,
		  600, 1.062200000000, 0.631000000000, 0.000800000000,
		  605, 1.045600000000, 0.566800000000, 0.000600000000,
		  610, 1.002600000000, 0.503000000000, 0.000340000000,
		  615, 0.938400000000, 0.441200000000, 0.000240000000,
		  620, 0.854449900000, 0.381000000000, 0.000190000000,
		  625, 0.751400000000, 0.321000000000, 0.000100000000,
		  630, 0.642400000000, 0.265000000000, 0.000049999990,
		  635, 0.541900000000, 0.217000000000, 0.000030000000,
		  640, 0.447900000000, 0.175000000000, 0.000020000000,
		  645, 0.360800000000, 0.138200000000, 0.000010000000,
		  650, 0.283500000000, 0.107000000000, 0.000000000000,
		  655, 0.218700000000, 0.081600000000, 0.000000000000,
		  660, 0.164900000000, 0.061000000000, 0.000000000000,
		  665, 0.121200000000, 0.044580000000, 0.000000000000,
		  670, 0.087400000000, 0.032000000000, 0.000000000000,
		  675, 0.063600000000, 0.023200000000, 0.000000000000,
		  680, 0.046770000000, 0.017000000000, 0.000000000000,
		  685, 0.032900000000, 0.011920000000, 0.000000000000,
		  690, 0.022700000000, 0.008210000000, 0.000000000000,
		  695, 0.015840000000, 0.005723000000, 0.000000000000,
		  700, 0.011359160000, 0.004102000000, 0.000000000000,
		  705, 0.008110916000, 0.002929000000, 0.000000000000,
		  710, 0.005790346000, 0.002091000000, 0.000000000000,
		  715, 0.004109457000, 0.001484000000, 0.000000000000,
		  720, 0.002899327000, 0.001047000000, 0.000000000000,
		  725, 0.002049190000, 0.000740000000, 0.000000000000,
		  730, 0.001439971000, 0.000520000000, 0.000000000000,
		  735, 0.000999949300, 0.000361100000, 0.000000000000,
		  740, 0.000690078600, 0.000249200000, 0.000000000000,
		  745, 0.000476021300, 0.000171900000, 0.000000000000,
		  750, 0.000332301100, 0.000120000000, 0.000000000000,
		  755, 0.000234826100, 0.000084800000, 0.000000000000,
		  760, 0.000166150500, 0.000060000000, 0.000000000000,
		  765, 0.000117413000, 0.000042400000, 0.000000000000,
		  770, 0.000083075270, 0.000030000000, 0.000000000000,
		  775, 0.000058706520, 0.000021200000, 0.000000000000,
		  780, 0.000041509940, 0.000014990000, 0.000000000000,
		  785, 0.000029353260, 0.000010600000, 0.000000000000,
		  790, 0.000020673830, 0.000007465700, 0.000000000000,
		  795, 0.000014559770, 0.000005257800, 0.000000000000,
		  800, 0.000010253980, 0.000003702900, 0.000000000000,
		  805, 0.000007221456, 0.000002607800, 0.000000000000,
		  810, 0.000005085868, 0.000001836600, 0.000000000000,
		  815, 0.000003581652, 0.000001293400, 0.000000000000,
		  820, 0.000002522525, 0.000000910930, 0.000000000000,
		  825, 0.000001776509, 0.000000641530, 0.000000000000,
		  830, 0.000001251141, 0.000000451810, 0.000000000000,
		};

		// The conversion matrix from XYZ to linear sRGB color spaces.
		// Values from https://en.wikipedia.org/wiki/SRGB.
		constexpr double XYZ_TO_SRGB[9] = {
		  +3.2406, -1.5372, -0.4986,
		  -0.9689, +1.8758, +0.0415,
		  +0.0557, -0.2040, +1.0570
		};

		constexpr int kLambdaMin = 360;
		constexpr int kLambdaMax = 830;

		static constexpr double kLambdaR = 680.0;
		static constexpr double kLambdaG = 550.0;
		static constexpr double kLambdaB = 440.0;

		double CieColorMatchingFunctionTableValue(double wavelength, int column) {
			if (wavelength <= kLambdaMin || wavelength >= kLambdaMax) {
				return 0.0;
			}
			double u = (wavelength - kLambdaMin) / 5.0;
			int row = static_cast<int>(std::floor(u));
			assert(row >= 0 && row + 1 < 95);
			assert(CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row] <= wavelength &&
				CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1)] >= wavelength);
			u -= row;
			return CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * row + column] * (1.0 - u) +
				CIE_2_DEG_COLOR_MATCHING_FUNCTIONS[4 * (row + 1) + column] * u;
		}

		double Interpolate(
			const std::vector<double>& wavelengths,
			const std::vector<double>& wavelength_function,
			double wavelength) {
			assert(wavelength_function.size() == wavelengths.size());
			if (wavelength < wavelengths[0]) {
				return wavelength_function[0];
			}
			for (unsigned int i = 0; i < wavelengths.size() - 1; ++i) {
				if (wavelength < wavelengths[i + 1]) {
					double u =
						(wavelength - wavelengths[i]) / (wavelengths[i + 1] - wavelengths[i]);
					return
						wavelength_function[i] * (1.0 - u) + wavelength_function[i + 1] * u;
				}
			}
			return wavelength_function[wavelength_function.size() - 1];
		}

		// The returned constants are in lumen.nm / watt.
		void ComputeSpectralRadianceToLuminanceFactors(
			const std::vector<double>& wavelengths,
			const std::vector<double>& solar_irradiance,
			double lambda_power, double* k_r, double* k_g, double* k_b) {
			*k_r = 0.0;
			*k_g = 0.0;
			*k_b = 0.0;
			double solar_r = Interpolate(wavelengths, solar_irradiance, kLambdaR);
			double solar_g = Interpolate(wavelengths, solar_irradiance, kLambdaG);
			double solar_b = Interpolate(wavelengths, solar_irradiance, kLambdaB);
			int dlambda = 1;
			for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
				double x_bar = CieColorMatchingFunctionTableValue(lambda, 1);
				double y_bar = CieColorMatchingFunctionTableValue(lambda, 2);
				double z_bar = CieColorMatchingFunctionTableValue(lambda, 3);
				const double* xyz2srgb = XYZ_TO_SRGB;
				double r_bar =
					xyz2srgb[0] * x_bar + xyz2srgb[1] * y_bar + xyz2srgb[2] * z_bar;
				double g_bar =
					xyz2srgb[3] * x_bar + xyz2srgb[4] * y_bar + xyz2srgb[5] * z_bar;
				double b_bar =
					xyz2srgb[6] * x_bar + xyz2srgb[7] * y_bar + xyz2srgb[8] * z_bar;
				double irradiance = Interpolate(wavelengths, solar_irradiance, lambda);
				*k_r += r_bar * irradiance / solar_r *
					pow(lambda / kLambdaR, lambda_power);
				*k_g += g_bar * irradiance / solar_g *
					pow(lambda / kLambdaG, lambda_power);
				*k_b += b_bar * irradiance / solar_b *
					pow(lambda / kLambdaB, lambda_power);
			}
			*k_r *= MAX_LUMINOUS_EFFICACY * dlambda;
			*k_g *= MAX_LUMINOUS_EFFICACY * dlambda;
			*k_b *= MAX_LUMINOUS_EFFICACY * dlambda;
		}

		void drawFullScreenQuad(const std::vector<bool>& enable_blend, Ref<VertexArray> vertex_array) {
			
			for (unsigned int i = 0; i < enable_blend.size(); ++i) {
				//std::cout << "Blend: " << enable_blend[i] << '\n';
				if (enable_blend[i]) {
					glEnablei(GL_BLEND, i);
				}
			}

			// shader bound outside of this
			vertex_array->bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			vertex_array->unbind();

			for (unsigned int i = 0; i < 4; ++i) {
				glDisablei(GL_BLEND, i);
			}
		}

		Atmosphere::Atmosphere(const std::vector<double>& wavelenghts, const std::vector<double>& solar_irradiance,
			double sun_angular_radius, double bottom_radius, double top_radius,
			const std::vector<DensityProfileLayer>& rayleigh_density, const std::vector<double>& rayleigh_scattering,
			const std::vector<DensityProfileLayer>& mie_density, const std::vector<double>& mie_scattering, const std::vector<double>& mie_extinction,
			double mie_phase_function_g,
			const std::vector<DensityProfileLayer>& absorbtion_density, const std::vector<double>& absorbtion_extinction,
			const std::vector<double>& ground_albedo,
			double max_sun_zenith_angle, double length_unit_in_meters, uint32_t num_precomputed_wavelengths) {


				m_num_precomputed_wavelengts = num_precomputed_wavelengths;
				bool precompute_illuminance = num_precomputed_wavelengths > 3;
				double sky_k_r, sky_k_g, sky_k_b;
				if (precompute_illuminance) {
					sky_k_r = sky_k_b = sky_k_g = MAX_LUMINOUS_EFFICACY;
				}
				else {
					ComputeSpectralRadianceToLuminanceFactors(wavelenghts, solar_irradiance, -3, &sky_k_r, &sky_k_g, &sky_k_b);
				}

				double sun_k_r, sun_k_g, sun_k_b;
				ComputeSpectralRadianceToLuminanceFactors(wavelenghts, solar_irradiance, 0, &sun_k_r, &sun_k_g, &sun_k_b);

				m_transmittance_tex = Texture2D::Create(TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, GL_RGBA32F, GL_RGBA);
				m_scattering_tex = Texture3D::Create(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, GL_RGBA32F, GL_RGBA);
				m_single_mie_scattering_tex = Texture3D::Create(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, GL_RGB32F, GL_RGBA);
				m_irradiance_tex = Texture2D::Create(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, GL_RGBA32F, GL_RGBA);
		
				// creating the shader
				// Here they create a portion of the shader with constants and functions for later use with other shaders
				// I am going to create the shader files with everything
				float vertices[] = {
				  -1.0, -1.0,
				  +1.0, -1.0,
				  -1.0, +1.0,
				  +1.0, +1.0,
				};

				m_quad_vao = VertexArray::Create();
				m_quad_vb = VertexBuffer::Create(vertices, sizeof(float) * 8);

				m_quad_vb->setLayout({
					{ ShaderDataType::Float2, "a_pos" },
					});
				m_quad_vao->setVertexBuffer(m_quad_vb);

				compute_transmittance = Shader::Create("compute_transmittance", "Shaders/compute_transmittance.vert", "Shaders/compute_transmittance_pre.frag");
				compute_direct_irradiance = Shader::Create("compute_direct_irradiance", "Shaders/compute_direct_irradiance.vert", "Shaders/compute_direct_irradiance.frag");
				compute_single_scattering = Shader::Create("compute_single_scattering", "Shaders/compute_single_scattering.vert", "Shaders/compute_geometry_shader.geom", "Shaders/compute_single_scattering.frag");
				compute_scattering_density = Shader::Create("compute_scattering_density", "Shaders/compute_scattering_density.vert", "Shaders/compute_geometry_shader.geom", "Shaders/compute_scattering_density.frag");
				compute_indirect_irradiance = Shader::Create("compute_indirect_irradiance", "Shaders/compute_indirect_irradiance.vert", "Shaders/compute_indirect_irradiance.frag");
				compute_multiple_scattering = Shader::Create("compute_multiple_scattering", "Shaders/compute_multiple_scattering.vert", "Shaders/compute_geometry_shader.geom", "Shaders/compute_multiple_scattering.frag");

				luminance_matrix_ubo = UniformBuffer::Create(sizeof(glm::mat4), 31);
				layer_ubo = UniformBuffer::Create(sizeof(int), 30);
				scattering_order_ubo = UniformBuffer::Create(sizeof(int), 29);
		}

		Atmosphere::~Atmosphere() {
		
		}

		void Atmosphere::init(uint32_t num_scattering_orders) {
			Ref<Texture2D> delta_irradiance			 = Texture2D::Create(IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT, GL_RGBA32F, GL_RGBA);
			Ref<Texture3D> delta_rayleigh_scattering = Texture3D::Create(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, GL_RGBA32F, GL_RGBA);
			Ref<Texture3D> delta_mie_scattering	     = Texture3D::Create(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, GL_RGBA32F, GL_RGBA);
			Ref<Texture3D> delta_scattering_density	 = Texture3D::Create(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, GL_RGBA32F, GL_RGBA);
			//Ref<Texture3D> delta_multiple_scattering = Texture3D::Create(SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT, SCATTERING_TEXTURE_DEPTH, GL_RGBA32F, GL_RGBA);

			Ref<Texture3D> delta_multiple_scattering = delta_rayleigh_scattering;
			GLuint fbo;
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "FBO incomplete!" << std::endl;

			if (m_num_precomputed_wavelengts <= 3) {
				glm::vec3 lambdas = glm::vec3(kLambdaR, kLambdaG, kLambdaB);
				glm::mat4 luminance_from_radiance = glm::mat4(1.0);
				precompute(fbo, delta_irradiance, delta_rayleigh_scattering, delta_mie_scattering, 
					delta_scattering_density, delta_multiple_scattering, lambdas, luminance_from_radiance, false, num_scattering_orders);
			}
			else {
				constexpr double lambda_min = 360.0;
				constexpr double lambda_max = 830.0;
				int num_iterations = (m_num_precomputed_wavelengts + 2) / 3;
				double dlambda = (lambda_max - lambda_min) / (3 * num_iterations);
				for (int i = 0; i < num_iterations; i++) {
					glm::vec3 lambdas = glm::vec3(
						lambda_min + (3 * i + 0.5) * dlambda,
						lambda_min + (3 * i + 1.5) * dlambda,
						lambda_min + (3 * i + 2.5) * dlambda
					);

					auto coeff = [dlambda](double lambda, int component) {
						double x = CieColorMatchingFunctionTableValue(lambda, 1);
						double y = CieColorMatchingFunctionTableValue(lambda, 2);
						double z = CieColorMatchingFunctionTableValue(lambda, 3);

						return static_cast<float>((
							XYZ_TO_SRGB[component * 3] * x +
							XYZ_TO_SRGB[component * 3 + 1] * y +
							XYZ_TO_SRGB[component * 3 + 2] * z) * dlambda);
						};

					glm::mat3 luminance_from_radiance{
						coeff(lambdas[0], 0), coeff(lambdas[1], 0), coeff(lambdas[2], 0),
						coeff(lambdas[0], 1), coeff(lambdas[1], 1), coeff(lambdas[2], 1),
						coeff(lambdas[0], 2), coeff(lambdas[1], 2), coeff(lambdas[2], 2)
					};

					precompute(fbo, delta_irradiance, delta_rayleigh_scattering, delta_mie_scattering,
						delta_scattering_density, delta_multiple_scattering, lambdas, luminance_from_radiance, i > 0, num_scattering_orders);
				}

				Ref<Shader> compute_transmittance = Shader::Create("compute_tran", "Shaders/compute_transmittance.vert", "Shaders/compute_transmittance_final.frag");
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_transmittance_tex->getRendererID(), 0);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);
				compute_transmittance->bind();
				drawFullScreenQuad({}, m_quad_vao);
			}

			
		}

		void Atmosphere::ConvertSpectrumToLinearSrgb(
			const std::vector<double>& wavelengths,
			const std::vector<double>& spectrum,
			double* r, double* g, double* b) {
			double x = 0.0;
			double y = 0.0;
			double z = 0.0;
			const int dlambda = 1;
			for (int lambda = kLambdaMin; lambda < kLambdaMax; lambda += dlambda) {
				double value = Interpolate(wavelengths, spectrum, lambda);
				x += CieColorMatchingFunctionTableValue(lambda, 1) * value;
				y += CieColorMatchingFunctionTableValue(lambda, 2) * value;
				z += CieColorMatchingFunctionTableValue(lambda, 3) * value;
			}
			*r = MAX_LUMINOUS_EFFICACY *
				(XYZ_TO_SRGB[0] * x + XYZ_TO_SRGB[1] * y + XYZ_TO_SRGB[2] * z) * dlambda;
			*g = MAX_LUMINOUS_EFFICACY *
				(XYZ_TO_SRGB[3] * x + XYZ_TO_SRGB[4] * y + XYZ_TO_SRGB[5] * z) * dlambda;
			*b = MAX_LUMINOUS_EFFICACY *
				(XYZ_TO_SRGB[6] * x + XYZ_TO_SRGB[7] * y + XYZ_TO_SRGB[8] * z) * dlambda;
		}


		void Atmosphere::precompute(uint32_t fbo, Ref<Texture2D> delta_irradiance, Ref<Texture3D> delta_rayleigh_scattering,
			Ref<Texture3D> delta_mie_scattering, Ref<Texture3D> delta_scattering_density, Ref<Texture3D> delta_multiple_scattering,
			const glm::vec3& lambdas, const glm::mat4& luminance_from_radiance, bool blend, uint32_t num_scattering_orders) {


			

			const GLuint draw_buffers[4] = {
				GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
			};

			glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
			glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

			// Compute the transmittance, and store it in m_transmittance_tex
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_transmittance_tex->getRendererID(), 0);
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
			glViewport(0, 0, TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT);

			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "FBO incomplete!" << std::endl;

			compute_transmittance->bind();
			drawFullScreenQuad({}, m_quad_vao);

			int width = 0, height = 0, depth = 0;
			glBindTexture(GL_TEXTURE_2D, m_transmittance_tex->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
			std::vector<float> transmittanceData(width * height * 4);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, transmittanceData.data());
			float sumTran = 0.0f;
			for (float v : transmittanceData) sumTran += v;
			std::cout << "Transmittance tex sum = " << sumTran << std::endl;


			// Compute the direct irradiance, store it in delta_irradiance_texture and,
			// depending on 'blend', either initialize irradiance_texture_ with zeros or
			// leave it unchanged (we don't want the direct irradiance in
			// irradiance_texture_, but only the irradiance from the sky).
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_irradiance->getRendererID(), 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_irradiance_tex->getRendererID(), 0);
			glDrawBuffers(2, draw_buffers);
			glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
			// bind uniforms and all
			compute_direct_irradiance->bind();
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, m_transmittance_tex->getRendererID());
			compute_direct_irradiance->setUniformInt("transmittance_texture", 0);
			drawFullScreenQuad({false, blend}, m_quad_vao);

			
			glBindTexture(GL_TEXTURE_2D,  m_irradiance_tex->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
			std::vector<float> directIrradianceData(width * height * 4);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, directIrradianceData.data());
			float sumDirIrr = 0.0f;
			for (float v : directIrradianceData) sumDirIrr += v;
			std::cout << "Direct Irradiance tex sum = " << sumDirIrr << std::endl;

			// Compute the rayleigh and mie single scattering, store them in
			// delta_rayleigh_scattering_texture and delta_mie_scattering_texture, and
			// either store them or accumulate them in scattering_texture_ and
			// optional_single_mie_scattering_texture_.
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_rayleigh_scattering->getRendererID(), 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, delta_mie_scattering->getRendererID(), 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, m_scattering_tex->getRendererID(), 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, m_single_mie_scattering_tex->getRendererID(), 0);
			glDrawBuffers(4, draw_buffers);
			glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);


			// bind uniforms and all
			compute_single_scattering->bind();
			luminance_matrix_ubo->setData(&luminance_from_radiance[0][0], sizeof(glm::mat4));
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, m_transmittance_tex->getRendererID());
			compute_single_scattering->setUniformInt("transmittance_texture", 0);
			for (int layer = 0; layer < SCATTERING_TEXTURE_DEPTH; layer++) {
				// bind layer uniform geometry shader
				layer_ubo->setData(&layer, sizeof(int));
				drawFullScreenQuad({ false, false, blend, blend }, m_quad_vao);
			}

			glBindTexture(GL_TEXTURE_3D, m_single_mie_scattering_tex->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
			std::vector<float> mieData(width * height * depth * 4);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, mieData.data());
			float sumMie = 0.0f;
			for (float v : mieData) sumMie += v;
			std::cout << "Mie scattering tex sum = " << sumMie << std::endl;

			glBindTexture(GL_TEXTURE_3D, m_scattering_tex->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
			std::vector<float> irradianceData(width * height * depth * 4);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, irradianceData.data());
			float sumIrr = 0.0f;
			for (float v : irradianceData) sumIrr += v;
			std::cout << "Single Scattering tex sum = " << sumIrr << std::endl;


			glBindTexture(GL_TEXTURE_3D, delta_rayleigh_scattering->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
			std::vector<float> rayyleighData(width * height * depth * 4);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, rayyleighData.data());
			float sumrayy = 0.0f;
			for (float v : rayyleighData) sumrayy += v;
			std::cout << "Delta Rayleigh scattering tex sum = " << sumrayy << std::endl;

			glBindTexture(GL_TEXTURE_3D, delta_mie_scattering->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
			std::vector<float> mieeData(width * height * depth * 4);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, mieeData.data());
			float sumMiee = 0.0f;
			for (float v : mieeData) sumMiee += v;
			std::cout << "Delta Mie scattering tex sum = " << sumMiee << std::endl;


			// multiple scattering, 2nd, 3rd and 4th order of scattering sequentially
 			for (int scattering_order = 2; scattering_order <= num_scattering_orders; scattering_order++) {
				// compute scattering density
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_scattering_density->getRendererID(), 0);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
				// bind texture uniforms and all
				compute_scattering_density->bind();

				glActiveTexture(GL_TEXTURE0 + 0);
				glBindTexture(GL_TEXTURE_2D, m_transmittance_tex->getRendererID());
				compute_scattering_density->setUniformInt("transmittance_texture", 0);
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_3D, delta_rayleigh_scattering->getRendererID());
				compute_scattering_density->setUniformInt("single_rayleigh_scattering_texture", 1);
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_3D, delta_mie_scattering->getRendererID());
				compute_scattering_density->setUniformInt("single_mie_scattering_texture", 2);
				glActiveTexture(GL_TEXTURE0 + 3);
				glBindTexture(GL_TEXTURE_3D, delta_multiple_scattering->getRendererID());
				compute_scattering_density->setUniformInt("multiple_scattering_texture", 3);
				glActiveTexture(GL_TEXTURE0 + 4);
				glBindTexture(GL_TEXTURE_2D, delta_irradiance->getRendererID());
				compute_scattering_density->setUniformInt("irradiance_texture", 4);

				int orderr = scattering_order;
				scattering_order_ubo->setData(&orderr, sizeof(int));
				for (uint32_t layer = 0; layer < SCATTERING_TEXTURE_DEPTH; layer++) {
					// bind layer uniform geometry shader
					layer_ubo->setData(&layer, sizeof(int));
					//Renderer2D::drawFullScreenQuad({});
					drawFullScreenQuad({}, m_quad_vao);
				}

			
				// compute the indirect irradiance, store it in delta_irradiance_texture and accumulate in m_irradiance_tex
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_irradiance->getRendererID(), 0);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_irradiance_tex->getRendererID(), 0);
				glDrawBuffers(2, draw_buffers);
				glViewport(0, 0, IRRADIANCE_TEXTURE_WIDTH, IRRADIANCE_TEXTURE_HEIGHT);
				// bind uniforms
				compute_indirect_irradiance->bind();
				luminance_matrix_ubo->setData(glm::value_ptr(luminance_from_radiance), sizeof(glm::mat4));
				//delta_rayleigh_scattering->bind(0);
				glActiveTexture(GL_TEXTURE0 + 0);
				glBindTexture(GL_TEXTURE_3D, delta_rayleigh_scattering->getRendererID());
				compute_indirect_irradiance->setUniformInt("single_rayleigh_scattering_texture", 0);
				//delta_mie_scattering->bind(1);
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_3D, delta_mie_scattering->getRendererID());
				compute_indirect_irradiance->setUniformInt("single_mie_scattering_texture", 1);
				//delta_multiple_scattering->bind(2);
				glActiveTexture(GL_TEXTURE0 + 2);
				glBindTexture(GL_TEXTURE_3D, delta_multiple_scattering->getRendererID());
				compute_indirect_irradiance->setUniformInt("multiple_scattering_texture", 2);
				int orderr1 = scattering_order - 1;
				scattering_order_ubo->setData(&orderr1, sizeof(int));
				//Renderer2D::drawFullScreenQuad({ false, true });
				drawFullScreenQuad({ false, true }, m_quad_vao);

				

				// compute multiple scattering, delta_multiple_scattering and accumulate in m_scattering_tex
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, delta_multiple_scattering->getRendererID(), 0);
				glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_scattering_tex->getRendererID(), 0);
				glDrawBuffers(2, draw_buffers);
				glViewport(0, 0, SCATTERING_TEXTURE_WIDTH, SCATTERING_TEXTURE_HEIGHT);
				// bind uniforms
				compute_multiple_scattering->bind();
				luminance_matrix_ubo->setData(glm::value_ptr(luminance_from_radiance), sizeof(glm::mat4));
				//m_transmittance_tex->bind(0);
				glActiveTexture(GL_TEXTURE0 + 0);
				glBindTexture(GL_TEXTURE_2D, m_transmittance_tex->getRendererID());
				compute_multiple_scattering->setUniformInt("transmittance_texture", 0);
				//delta_scattering_density->bind(1);
				glActiveTexture(GL_TEXTURE0 + 1);
				glBindTexture(GL_TEXTURE_3D, delta_scattering_density->getRendererID());
				compute_multiple_scattering->setUniformInt("scattering_density_texture", 1);
				for (uint32_t layer = 0; layer < SCATTERING_TEXTURE_DEPTH; layer++) {
					// bind layer uniform geometry shader
					layer_ubo->setData(&layer, sizeof(int));
					//Renderer2D::drawFullScreenQuad({});
					drawFullScreenQuad({false, true}, m_quad_vao);
				}

				
			}

			glBindTexture(GL_TEXTURE_2D, m_irradiance_tex->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
			std::vector<float> indirectIrradianceData(width * height * 4);
			glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, indirectIrradianceData.data());
			float sumIndirIrr = 0.0f;
			for (float v : indirectIrradianceData) sumIndirIrr += v;
			std::cout << "Indirect Irradiance tex sum = " << sumIndirIrr << std::endl;
			

			glBindTexture(GL_TEXTURE_3D, m_scattering_tex->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
			std::vector<float> mulData(width * height * depth * 4);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, mulData.data());
			float sumMul = 0.0f;
			for (float v : mulData) sumMul += v;
			std::cout << "Multiple scattering tex sum = " << sumMul << std::endl;

			// Optional for single_mie_scattering
			/*glBindTexture(GL_TEXTURE_3D, m_scattering_tex->getRendererID());
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_WIDTH, &width);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_HEIGHT, &height);
			glGetTexLevelParameteriv(GL_TEXTURE_3D, 0, GL_TEXTURE_DEPTH, &depth);
			std::vector<float> mieData(width * height * depth * 4);
			glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, mieData.data());
			float sumMie = 0.0f;
			for (float v : mieData) sumMie += v;
			std::cout << "Mie scattering tex sum = " << sumMie << std::endl;*/

			//// Bind the texture
			//m_transmittance_tex->bind();

			//// Allocate buffer to read floats
			//std::vector<float> pixels(TRANSMITTANCE_TEXTURE_WIDTH * TRANSMITTANCE_TEXTURE_HEIGHT * 4); // RGBA32F

			//glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels.data());
			//glBindTexture(GL_TEXTURE_2D, 0);

			//// Save as EXR (OpenEXR HDR format)
			//if (!stbi_write_hdr("output3.exr", TRANSMITTANCE_TEXTURE_WIDTH, TRANSMITTANCE_TEXTURE_HEIGHT, 4, pixels.data())) {z
			//	std::cout << "Failed to save EXR!" << std::endl;
			//}
			//else {
			//	std::cout << "Saved HDR texture to output.exr" << std::endl;
			//}

			//width = SCATTERING_TEXTURE_WIDTH;
			//height = SCATTERING_TEXTURE_HEIGHT;

			//std::vector<float> pixels(width* height * 4); // RGBA16F / RGBA32F

			//glGetTextureSubImage(
			//	m_single_mie_scattering_tex->getRendererID(),   // GLuint scattering texture
			//	0,                     // mip level
			//	0, 0, 0,               // x, y, z offset → z=0 (first slice)
			//	width,
			//	height,
			//	1,                     // depth = 1 slice
			//	GL_RGBA,
			//	GL_FLOAT,
			//	pixels.size() * sizeof(float),
			//	pixels.data()
			//);

			//stbi_write_hdr("scattering_slice_z0.hdr",
			//	width, height, 4, pixels.data());
			

			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, 0, 0);
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, 0, 0);
		}
		
}   //  namespace iara